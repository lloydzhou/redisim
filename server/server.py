import asyncio
import logging
import json
import redisim
import redis.asyncio as redis
# import redis

import tornado.log
import tornado.web
import tornado.ioloop
import tornado.autoreload
import tornado.httpserver
import tornado.websocket
from os.path import abspath, dirname, join
from config import options, parse_command_line

from core.route import route, routes


redis_url = "redis://%s:%s" % (options.REDIS_HOST, options.REDIS_PORT)
pool = redis.ConnectionPool.from_url(redis_url, decode_responses=True)
client = redis.Redis(connection_pool=pool)
# module=False, using lua script
# module=True, using redis module
im = client.im(module=False)


def array_to_dict(args, **kwargs):
    if isinstance(args, (list, tuple)):
        kwargs.update(dict(args[i:i+2] for i in range(0, len(args), 2)))
    elif isinstance(args, dict):
        kwargs.update(args)
    return kwargs


async def login(user_id, passwd=None, *args, **kwargs):
    await im.user(user_id, **kwargs)
    res = await im.user(user_id)
    return array_to_dict(res, uid=user_id)


async def recive(user_id, **kwargs):
    # return mid, uid, tuid, gid, message
    res = await im.recive(user_id, **kwargs)
    if isinstance(res, list):
        channel, messages = res[0]
        # print('channel', channel)
        for mid, message in messages:
            m = array_to_dict(message, tuid=channel[2:])
            if 'FW' in m:
                fwt, fwc = m['FW'].split(':')
                mr = await im.message(fwc, mid, group=fwt == 'gs')
                if mr:
                    mid, message = mr
                    if 'gs' == fwt:
                        m = array_to_dict(message)
                    else:
                        m = array_to_dict(message)
                    yield mid, m['uid'], fwc, fwc if 'gs' == fwt else '', m
            else:
                yield mid, m['uid'], channel[2:], '', m
    yield None, None, None, None, None
    await asyncio.sleep(0.1)


async def im_action(command, *params):
    return await getattr(im, command)(*params)


parse_command_line()
tornado.log.enable_pretty_logging()


@route(r"/ws")
class WebSocketHandler(tornado.websocket.WebSocketHandler):

    async def open(self, *args, **kwargs):
        # print("WebSocket opened", self, self.request, self.request.headers, args, kwargs)
        user_id = self.get_argument('user_id')

        user = await login(user_id, name='user1')

        self.write_message(json.dumps(user))

        async def loop():
            start = self.get_argument('last_message_id') or 0
            while self.ws_connection and not self.ws_connection.is_closing():
                async for mid, uid, tuid, gid, message in recive(user_id, block=10000, count=10, start=start):
                    if mid and mid != start:
                        start = mid
                        # print(mid, message)
                        self.write_message({
                            'id': mid,
                            'uid': uid,
                            'tuid': tuid,
                            'gid': gid,
                            'message': message,
                        })
            # print(mid)
        # do not block cpu
        asyncio.ensure_future(loop())

    async def on_message(self, message):
        # print("WebSocket message", message)
        # self.write_message(u"You said: " + message)
        try:
            message = json.loads(message)
            if 'action' in message:
                action, id, params = message['action'], message.get('id'), message.get('params', [])
                if action.upper() in ['SEND', 'GSEND', 'USER', 'GROUP', 'LINK', 'UNLINK', 'JOIN', 'QUIT']:
                    response = await im_action(action, *params)
                    self.write_message({'id': id, 'response': response})
                    # print('write_message', id, response)
        except Exception as e:
            logging.error('error parse message %s %r', message, e)

    def on_close(self, *args, **kwargs):
        print("WebSocket closed", self, args, kwargs)


if __name__ == "__main__":
    # from modules import *
    root_path = dirname(dirname(abspath(__file__)))
    path = join(root_path, "web", "dist")

    @route(r"/(chat|mine|faxian|address|login)(.*)")
    class RedirectHandler(tornado.web.RequestHandler):
        def get(self, *args):
            self.redirect('/')

    @route(r"/(.*)", path=path, default_filename="index.html")
    class StaticFileHandler(tornado.web.StaticFileHandler):
        pass

    application = tornado.web.Application(routes(), **dict(
        debug=options.DEBUG,
    ))
    server = tornado.httpserver.HTTPServer(application, max_buffer_size=1048576000)
    server.listen(port=options.SERVER_PORT)
    server.start()
    logging.info("Start Success: 0.0.0.0:{}".format(options.SERVER_PORT))

    tornado.ioloop.IOLoop.instance().start()

