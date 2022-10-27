import logging
import aioredis
import operator
from functools import reduce
from tornado.options import options


async def get_redis_pool():
    try:
        return await aioredis.from_url(
            "redis://%s:%s" % (options.REDIS_HOST, options.REDIS_PORT),
            encoding='utf-8',
            decode_responses=True,
        )
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
    res = await pool.execute_command("IM.USER", user_id, *merge_args(*args, **kwargs))
    res = await pool.execute_command("IM.USER", user_id)
    return array_to_dict(*res, uid=user_id)


async def recive(user_id, **kwargs):
    # return mid, uid, tuid, gid, message
    pool = await get_redis_pool()
    res = await pool.execute_command("IM.RECIVE", *merge_args(user_id, **kwargs))
    if isinstance(res, list):
        channel, messages = res[0]
        # print('channel', channel)
        for mid, message in messages:
            m = array_to_dict(*message, tuid=channel[2:])
            if 'FW' in m:
                fwt, fwc = m['FW'].split(':')
                mr = await pool.execute_command("IM.MESSAGE", 'GROUP' if fwt == 'gs' else 'USER', fwc, mid)
                if mr:
                    mid, message = mr
                    if 'gs' == fwt:
                        m = array_to_dict(*message)
                    else:
                        m = array_to_dict(*message)
                    yield mid, m['uid'], fwc, fwc if 'gs' == fwt else '', m
            else:
                yield mid, m['uid'], channel[2:], '', m
    yield None, None, None, None, None


async def action(command, *params):
    pool = await get_redis_pool()
    return await pool.execute_command(command, *params)

