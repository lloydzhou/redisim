from os.path import dirname, join, isfile
from tornado.options import define, parse_config_file


def load_config():
    define('DEBUG', default=True)
    define('SERVER_PORT', default=8888)

    define('REDIS_HOST', default="localhost")
    define('REDIS_PORT', default=6379)

    root_path = dirname(dirname(__file__))

    config_file = join(root_path, "etc", "web_config.conf")
    if isfile(config_file):
        parse_config_file(config_file)

