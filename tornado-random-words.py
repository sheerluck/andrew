from random import choice

import tornado.ioloop
import tornado.web


class MainHandler(tornado.web.RequestHandler):

    def initialize(self, c) -> None:
        self.content = c

    def get(self):
        s = " ".join([choice(self.content) for _ in range(3)])
        self.write(s)
        print(s)


if __name__ == "__main__":
    name = "google-10000-english.txt"
    with open(name) as f:
        content = [line.rstrip('\n') for line in f]
    app = tornado.web.Application([(r"/", MainHandler, dict(c=content))])
    app.listen(8888)
    try:
        tornado.ioloop.IOLoop.current().start()
    except KeyboardInterrupt:
        tornado.ioloop.IOLoop.current().stop()
