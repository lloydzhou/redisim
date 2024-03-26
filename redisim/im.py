import asyncio
from time import time
from .util import merge_args
from .module import IMModule


class IM(IMModule):

    def __init__(self, client):
        self.client = client
        self.execute_command = client.execute_command
        self.recive_script = self.client.register_script(
            """
            local key = KEYS[1]
            local res = redis.call("XRANGE", KEYS[1], ARGV[1], ARGV[1])
            if res ~= nil and #res == 1 then
                return res[1]
            end
            return nil
            """
        )
        self.message_script = self.client.register_script(
            """
            local key = KEYS[1]
            local res = redis.call("XRANGE", KEYS[1], ARGV[1], ARGV[1])
            if res ~= nil and #res == 1 then
                return res[1]
            end
            return nil
            """
        )
        self.send_script = self.client.register_script(
            """
            local s_to_user_id = KEYS[1]
            local s_user_id = KEYS[2]
            local r_to_user_id = KEYS[3]
            local user_id = table.remove(ARGV, 1)
            local mid = redis.call("XADD", s_to_user_id, "*", "uid", user_id, unpack(ARGV))
            redis.call("XADD", s_user_id, mid, "FW", s_to_user_id)
            redis.call("ZADD", r_to_user_id, "INCR", 1, s_user_id)
            return mid
            """
        )
        self.gsend_script = self.client.register_script(
            # 这里只能确定两个key, 剩下的key是从smembers读取的
            """
            local s_group_id = KEYS[1]
            local m_group_id = KEYS[2]
            local s_user_id = table.remove(ARGV, 1)
            local mid = redis.call("XADD", s_group_id, "*", "uid", s_user_id, unpack(ARGV))
            local members = redis.call("SMEMBERS", m_group_id)
            for i, uid in ipairs(members) do
                redis.call("XADD", "s:" .. uid, mid, "FW", s_group_id)
                if uid ~= s_user_id then
                    redis.call("ZADD", "r:" .. uid, "INCR", 1, s_group_id)
                end
            end
            return mid
            """
        )
        self.join_script = self.client.register_script(
            """
            local c_user_id = KEYS[1]
            local r_user_id = KEYS[2]
            local m_group_id = KEYS[3]
            local s_group_id = KEYS[4]
            local user_id = ARGV[1]
            local group_id = ARGV[2]
            local timestamp, ms = unpack(redis.call("TIME"))
            -- 1. add user contact;
            redis.call("ZADD", c_user_id, timestamp .. "000", group_id)
            -- 2. add group member;
            redis.call("SADD", m_group_id, user_id)
            -- 3. send join group message;
            local mid = redis.call("XADD", s_group_id, "*", "action", "join", "uid", user_id, "gid", group_id)
            local members = redis.call("SMEMBERS", m_group_id)
            for i, uid in ipairs(members) do
                redis.call("XADD", "s:" .. uid, mid, "FW", s_group_id)
                if uid ~= user_id then
                    redis.call("ZADD", "r:" .. uid, "INCR", 1, s_group_id)
                end
            end
            """
        )
        self.quit_script = self.client.register_script(
            """
            local c_user_id = KEYS[1]
            local r_user_id = KEYS[2]
            local m_group_id = KEYS[3]
            local s_group_id = KEYS[4]
            local user_id = ARGV[1]
            local group_id = ARGV[2]
            -- 1. remove user contact;
            redis.call("ZREM", c_user_id, group_id)
            redis.call("ZREM", r_user_id, "s:" .. group_id)
            -- 2. remove user from group
            local res = redis.call("SREM", m_group_id, user_id)
            -- 3. send quit group message
            local mid = redis.call("XADD", s_group_id, "*", "action", "quit", "uid", user_id, "gid", group_id)
            local members = redis.call("SMEMBERS", m_group_id)
            for i, uid in ipairs(members) do
                redis.call("XADD", "s:" .. uid, mid, "FW", s_group_id)
            end
            return res
            """
        )
        self.group_info_script = self.client.register_script(
            """
            local i_group_id = KEYS[1]
            local m_group_id = KEYS[2]
            local info = redis.call("HGETALL", i_group_id)
            local members = redis.call("SMEMBERS", m_group_id)
            return { info, members }
            """
        )
        self.create_group_script = self.client.register_script(
            # 1. hmset group info; 2. join_script logic
            """
            local i_group_id = KEYS[1]
            local c_user_id = KEYS[2]
            local r_user_id = KEYS[3]
            local m_group_id = KEYS[4]
            local s_group_id = KEYS[5]
            local group_id = table.remove(ARGV, 1)
            local user_id = table.remove(ARGV, 1)
            local timestamp, ms = unpack(redis.call("TIME"))
            -- 1. hmset group info
            redis.call("HMSET", i_group_id, "master", user_id, unpack(ARGV))
            -- 2. add user contact;
            redis.call("ZADD", c_user_id, timestamp .. "000", group_id)
            -- 3. add group member;
            redis.call("SADD", m_group_id, user_id)
            -- 4. send join group message;
            local mid = redis.call("XADD", s_group_id, "*", "action", "join", "uid", user_id, "gid", group_id)
            local members = redis.call("SMEMBERS", m_group_id)
            for i, uid in ipairs(members) do
                redis.call("XADD", "s:" .. uid, mid, "FW", s_group_id)
                if uid ~= user_id then
                    redis.call("ZADD", "r:" .. uid, "INCR", 1, s_group_id)
                end
            end
            """
        )
        self.link_script = self.client.register_script(
            """
            local s_to_user_id = KEYS[1]
            local s_user_id = KEYS[2]
            local r_to_user_id = KEYS[3]
            local c_user_id = KEYS[4]
            local user_id = ARGV[1]
            local to_user_id = ARGV[2]
            local timestamp, ms = unpack(redis.call("TIME"))
            local res = redis.call("ZADD", c_user_id, timestamp, s_to_user_id)
            if res ~= 0 then
                local mid = redis.call("XADD", s_to_user_id, "*", "action", "link", "uid", user_id, "tuid", to_user_id)
                redis.call("XADD", s_user_id, mid, "FW", s_to_user_id)
                redis.call("ZADD", r_to_user_id, "INCR", 1, s_user_id)
            end
            return res
            """
        )
        self.unlink_script = self.client.register_script(
            """
            local c_user_id = KEYS[1]
            local r_user_id = KEYS[2]
            local to_user_id = ARGV[1]
            redis.call("ZREM", r_user_id, "s:" .. to_user_id)
            return redis.call("ZREM", c_user_id, to_user_id)
            """
        )

    def recive(self, user_id, block=10000, count=20, start=0):
        res = self.execute_command("XREAD", "COUNT", count, "BLOCK", block, "STREAMS", f"s:{user_id}", start)
        return res if isinstance(res, list) and len(res) > 0 else None

    def message(self, key, message_id, group=False):
        return self.message_script(
            keys=[f"gs:{key}" if group else f"s:{key}"],
            args=[message_id]
        )

    def send(self, user_id, to_user_id, *args, **kwargs):
        return self.send_script(
            keys=[f"s:{to_user_id}", f"s:{user_id}", f"r:{to_user_id}"],
            args=merge_args(user_id, *args, **kwargs)
        )

    def gsend(self, user_id, group_id, *args, **kwargs):
        return self.gsend_script(
            keys=[f"gs:{group_id}", f"gm:{group_id}"],
            args=merge_args(user_id, *args, **kwargs)
        )

    def user(self, user_id, **kwargs):
        if kwargs:
            return self.execute_command(*merge_args("HMSET", f"u:{user_id}", **kwargs))
        return self.execute_command("HGETALL", f"u:{user_id}")

    def group(self, group_id, user_id=None, **kwargs):
        if kwargs:
            return self.create_group_script(
                keys=[f"gi:{group_id}", f"c:{user_id}", f"r:{user_id}", f"gm:{group_id}", f"gs:{group_id}"],
                args=merge_args(group_id, user_id, **kwargs)
            )
        return self.group_info_script(keys=[f"gi:{group_id}", f"gm:{group_id}"], args=[])

    def link(self, user_id, to_user_id):
        return self.link_script(
            keys=[f"s:{to_user_id}", f"s:{user_id}", f"r:{to_user_id}", f"c:{user_id}"],
            args=[user_id, to_user_id]
        )

    def unlink(self, user_id, to_user_id):
        return self.unlink_script(
            keys=[f"c:{user_id}", f"r:{user_id}"],
            args=[to_user_id]
        )

    def join(self, user_id, group_id):
        return self.join_script(
            keys=[f"c:{user_id}", f"r:{user_id}", f"gm:{group_id}", f"gs:{group_id}"],
            args=[user_id, group_id]
        )

    def quit(self, user_id, group_id):
        return self.quit_script(
            keys=[f"c:{user_id}", f"r:{user_id}", f"gm:{group_id}", f"gs:{group_id}"],
            args=[user_id, group_id]
        )


class AsyncIM(IM):

    async def recive(self, user_id, block=10000, count=20, start=0):
        res = await self.execute_command("XREAD", "COUNT", count, "BLOCK", block, "STREAMS", f"s:{user_id}", start)
        return res if isinstance(res, list) and len(res) > 0 else None

