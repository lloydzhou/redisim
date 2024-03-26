from redis.commands import RedisModuleCommands, AsyncRedisModuleCommands
from .module import *
from .im import *



def im(self, module=True):
    return IMModule(self) if module else IM(self)

def asyncim(self, module=True):
    return IMModule(self) if module else AsyncIM(self)


RedisModuleCommands.im = im
AsyncRedisModuleCommands.im = asyncim


__version__ = "0.0.2"
