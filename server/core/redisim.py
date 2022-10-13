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


def array_to_dict(*args, **kwargs):
    kwargs.update(dict(args[i:i+2] for i in range(0, len(args), 2)))
    return kwargs


async def login(user_id, passwd=None, *args, **kwargs):
    pool = await get_redis_pool()
    # TODO auth
    res = await pool.execute("IM.USER", user_id, *merge_args(*args, **kwargs))
    res = await pool.execute("IM.USER", user_id)
    return array_to_dict(*res, uid=user_id)


async def recive(user_id, **kwargs):
    pool = await get_redis_pool()
    res = await pool.execute("IM.RECIVE", *merge_args(user_id, **kwargs))
    if isinstance(res, list):
        channel, messages = res[0]
        # print('channel', channel)
        for mid, message in messages:
            m = array_to_dict(*message, tuid=channel[2:])
            if 'FW' in m:
                fw = m['FW']
                mr = await pool.execute("XRANGE", fw, mid, mid)
                if len(mr) > 0:
                    mid, message = mr[0]
                    if 'gs:' == fw[:3]:
                        m = array_to_dict(*message, gid=fw[3:])
                    else:
                        m = array_to_dict(*message, tuid=fw[2:], uid=channel[2:])
                    yield mid, m
            else:
                yield mid, m
    yield None, None


