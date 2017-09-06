"""
MicroCanister is a simple plugin for bottle, providing:

- formatted logs

All code was brazenly stolen from https://github.com/dagnelies/canister

"""
import sys
import logging
import logging.handlers
import bottle
import threading
import time


def _buildLogger(config):
    level = config.get('microcanister.log_level', 'INFO')
    log = logging.getLogger('microcanister')

    h = logging.StreamHandler()

    log.setLevel(level)
    msg = '%(asctime)s %(levelname)-8s [%(threadName)s]   %(message)s'
    f = logging.Formatter(msg)
    h.setFormatter(f)
    log.addHandler(h)

    return log


class MicroCanister:
    name = 'microcanister'

    def __init__(self):
        pass

    def setup(self, app):

        config = app.config

        log = _buildLogger(config)

        log.info('============')
        log.info('Initializing')
        log.info('============')

        log.info('python version: ' + sys.version)
        log.info('bottle version: ' + bottle.__version__)

        log.info('------------------------------------------')
        for k, v in app.config.items():
            log.info('%-30s = %s' % (k, v))
        log.info('------------------------------------------')

        self.app = app
        self.log = log
        app.log = log

    def apply(self, callback, route):

        log = self.log

        def wrapper(*args, **kwargs):

            start = time.time()

            req = bottle.request
            res = bottle.response

            threading.current_thread().name = req.remote_addr
            log.info(req.method + ' ' + req.url)

            result = callback(*args, **kwargs)

            elapsed = time.time() - start

            respair = (res.status_code, 1000 * elapsed)
            if elapsed > 1:
                log.warn('Response: %d (%dms !!!)' % respair)
            else:
                log.info('Response: %d (%dms)'     % respair)
            return result

        return wrapper

    def close(self):
        pass
