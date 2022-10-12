urls = []


def route(url, **kwargs):

    def handler_wrapper(handler):
        urls.append((url, handler, kwargs))
        return handler
    return handler_wrapper


def routes():
    return urls

