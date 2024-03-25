import operator
from functools import reduce
from redis.commands import RedisModuleCommands, AsyncRedisModuleCommands


def merge_args(*args, **kwargs):
    _kwargs = [(k, v) for k, v in kwargs.items() if v is not None]
    _kwargs = _kwargs if len(_kwargs) > 0 else [()]
    return operator.add(args, reduce(operator.add, _kwargs))


class IM:

    def __init__(self, client):
        self.client = client
        self.execute_command = client.execute_command

    def recive(self, user_id, block=10000, count=20, start=0):
        params = merge_args("IM.RECIVE", user_id, BLOCK=block, COUNT=count, START=start)
        return self.execute_command(*params)

    def message(self, key, message_id, group=False):
        params = ["IM.MESSAGE", "GROUP" if group else "USER", key, message_id]
        return self.execute_command(*params)

    def send(self, user_id, to_user_id, *args, **kwargs):
        params = merge_args("IM.SEND", user_id, to_user_id, *args, **kwargs)
        return self.execute_command(*params)

    def gsend(self, user_id, group_id, *args, **kwargs):
        params = merge_args("IM.GSEND", user_id, group_id, *args, **kwargs)
        return self.execute_command(*params)

    def user(self, user_id, **kwargs):
        params = merge_args("IM.USER", user_id, **kwargs)
        return self.execute_command(*params)

    def group(self, group_id, user_id=None, **kwargs):
        params = merge_args("IM.GROUP", group_id, user_id=user_id, **kwargs)
        return self.execute_command(*params)

    def link(self, user_id, to_user_id):
        params = merge_args("IM.LINK", user_id, to_user_id)
        return self.execute_command(*params)

    def unlink(self, user_id, to_user_id):
        params = merge_args("IM.UNLINK", user_id, to_user_id)
        return self.execute_command(*params)

    def join(self, user_id, group_id):
        params = merge_args("IM.JOIN", user_id, group_id)
        return self.execute_command(*params)

    def quit(self, user_id, group_id):
        params = merge_args("IM.QUIT", user_id, group_id)
        return self.execute_command(*params)


def im(self, script=False):
    # TODO using script
    return IM(self) if script else IM(self)


RedisModuleCommands.im = im
AsyncRedisModuleCommands.im = im


__version__ = "0.0.1"
