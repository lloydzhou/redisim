import asyncio
from time import time
from .util import merge_args
from .module import IMModule


class IM(IMModule):

    def __init__(self, client):
        self.client = client
        self.send_script = self.client.register_script(
            """
            local s_to_user_id = KEYS[1]
            local s_user_id = KEYS[2]
            local r_to_user_id = KEYS[3]
            local mid = redis.call("XADD", s_to_user_id, "*", unpack(ARGV))
            redis.call("XADD", s_user_id, mid, "FW", s_to_user_id)
            redis.call("ZADD", r_to_user_id, "INCR", 1, s_user_id)
            return mid
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

    def execute_command(self, command, *args, **kwargs):
        print('execute_command', command, args, kwargs)
        name = command.replace('IM.', '')
        if hasattr(self, name):
            return getattr(self, name)(*args, **kwargs)
        return self.client.execute_command(command, *args, **kwargs)

    def recive(self, user_id, block=10000, count=20, start=0):
        return self.execute_command("XREAD", "COUNT", count, "BLOCK", block, "STREAMS", f"s:{user_id}", start)

    def message(self, key, message_id, group=False):
        return self.message_script(
            keys=[f"gs:{key}" if group else f"s:{key}"],
            args=[message_id]
        )
    def send(self, user_id, to_user_id, *args, **kwargs):
        return self.send_script(
            keys=[f"s:{to_user_id}", f"s:{user_id}", f"r:{to_user_id}"],
            args=merge_args("uid", user_id, *args, **kwargs)
        )


class AsyncIM(IM):

    async def _message(self, key, message_id, group=False):
        res = await self.execute_command("XRANGE", f"gs:{key}" if group else f"s:{key}", message_id, message_id)
        return res[0] if isinstance(res, list) and len(res) > 0 else None
