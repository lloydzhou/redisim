import logging
import aioredis
import operator
from functools import reduce
from tornado.options import options
from aioredis.errors import ConnectionClosedError as ServerConnectionClosedError


async def get_redis_pool():
    try:
        return await aioredis.create_redis_pool(
            (options.REDIS_HOST, options.REDIS_PORT), encoding='utf-8')
    except ConnectionRefusedError as e:
        logging.error('%r cannot connect to redis on:', e, options.REDIS_HOST, options.REDIS_PORT)
        raise e


def merge_args(*args, **kwargs):
    args = operator.add(args, reduce(operator.add, [(k, v) for k, v in kwargs.items()]))
    return args


async def login(user_id, passwd=None, *args, **kwargs):
    pool = await get_redis_pool()
    # TODO auth
    res = await pool.execute("IM.USER", user_id, *merge_args(*args, **kwargs))
    print(res)
    res = await pool.execute("IM.USER", user_id)
    return res


async def recive(user_id, **kwargs):
    pool = await get_redis_pool()
    res = await pool.execute("IM.RECIVE", *merge_args(user_id, **kwargs))
    if isinstance(res, list):
        channel, messages = res[0]
        return messages
    return []


