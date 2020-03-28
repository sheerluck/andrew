from random import choice

import tornado.ioloop
import tornado.web


class MainHandler(tornado.web.RequestHandler):

    def initialize(self) -> None:
        self.filename = "google-10000-english.txt"
        with open(self.filename) as f:
            self.content = [line.rstrip('\n') for line in f]

    def get(self):
        s = " ".join([choice(self.content) for _ in range(3)])
        self.write(s)
        print(s)


if __name__ == "__main__":
    app = tornado.web.Application([(r"/", MainHandler)])
    app.listen(8888)
    try:
        tornado.ioloop.IOLoop.current().start()
    except KeyboardInterrupt:
        tornado.ioloop.IOLoop.current().stop()
