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
from core.redisim import login, recive, action as im_action

load_config()
parse_command_line()
tornado.log.enable_pretty_logging()


@route(r"/ws")
class WebSocketHandler(tornado.websocket.WebSocketHandler):

    async def open(self, *args, **kwargs):
        # print("WebSocket opened", self, self.request, self.request.headers, args, kwargs)
        user_id = self.get_argument('user_id')

        user = await login(user_id, name='user1')

        print(user_id, user)
        self.write_message(json.dumps(user))

        async def loop():
            start = self.get_argument('last_message_id') or 0
            while self.ws_connection and not self.ws_connection.is_closing():
                async for mid, uid, tuid, gid, message in recive(user_id, block=10000, count=10, start=start):
                    if mid:
                        start = mid
                        # print(mid, message)
                        self.write_message({
                            'id': mid,
                            'uid': uid,
                            'tuid': tuid,
                            'gid': gid,
                            'message': message,
                        })
            print(mid)
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
                    response = await im_action('IM.' + action.upper(), *params)
                    self.write_message({'id': id, 'response': response})
                    print(response)
        except Exception as e:
            logging.error('error parse message %s %r', message, e)

    def on_close(self, *args, **kwargs):
        print("WebSocket closed", self, args, kwargs)


if __name__ == "__main__":
    # from modules import *
    root_path = dirname(dirname(abspath(__file__)))
    path = join(root_path, "web", "dist")
    log_path = join(root_path, "logs")

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

