import asyncio
import logging
import json

import tornado.log
import tornado.web
import tornado.ioloop
import tornado.autoreload
import tornado.httpserver
import tornado.websocket
from os.path import abspath, dirname, join
from tornado.options import options, parse_command_line

from config import load_config
from core.route import route, routes
from core.redisim import login, recive

load_config()
parse_command_line()
tornado.log.enable_pretty_logging()


@route(r"/ws")
class WebSocketHandler(tornado.websocket.WebSocketHandler):

    async def open(self, *args, **kwargs):
        print("WebSocket opened", self, self.request, self.request.headers, args, kwargs)
        user_id = self.get_argument('user_id')

        user = await login(user_id, name='user1')

        print(user_id, user)
        self.write_message(json.dumps(user))

        async def loop():
            start = 0
            while self.ws_connection and not self.ws_connection.is_closing():
                async for mid, message in recive(user_id, block=10000, count=10, start=start):
                    if mid:
                        start = mid
                        # print(mid, message)
                        self.write_message({'id': mid, 'message': message})
            print(mid)
        # do not block cpu
        asyncio.ensure_future(loop())

    def on_message(self, message):
        print("WebSocket message", message)
        self.write_message(u"You said: " + message)
        # self.ping()

    def on_close(self, *args, **kwargs):
        print("WebSocket closed", self, args, kwargs)


@route(r"/api")
class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello, world")


if __name__ == "__main__":
    # from modules import *
    root_path = dirname(dirname(abspath(__file__)))
    path = join(root_path, "web", "build")
    log_path = join(root_path, "logs")

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

