from redis.commands import RedisModuleCommands, AsyncRedisModuleCommands
from .module import *
from .im import *


def im(self, module=True):
    return IMModule(self) if module else IM(self)

def asyncim(self, module=True):
    return IMModule(self) if module else AsyncIM(self)


RedisModuleCommands.im = im
AsyncRedisModuleCommands.im = asyncim


try:
    from redis.exceptions import NoScriptError
    from redis.asyncio.connection import DefaultParser as AsyncDefaultParser
    from redis.connection import DefaultParser
    # using kvrocks instead of redis
    AsyncDefaultParser.EXCEPTION_CLASSES['ERR']['NOSCRIPT No matching script. Please use EVAL'] = NoScriptError
    DefaultParser.EXCEPTION_CLASSES['ERR']['NOSCRIPT No matching script. Please use EVAL'] = NoScriptError
except Exception:
    pass


__version__ = "0.0.5"
