#-*- coding: utf-8  -*-

import sys
from PyQt5.QtCore import QObject, QCoreApplication, QThread, pyqtSignal, pyqtSlot


class Worker(QObject):

    finished = pyqtSignal()

    def __init__(self, a):
        super(Worker, self).__init__()
        self.m_a = a

    @pyqtSlot()
    def boo(self):
        print(self.m_a)
        self.finished.emit()


def connectAll(thread, worker):
    worker.finished.connect(thread.quit)
    thread.started.connect(worker.boo)
    thread.finished.connect(app.exit)
    # return (thread, worker)


app = QCoreApplication(sys.argv)

thread = QThread()
worker = Worker("Hello, World!")
worker.moveToThread(thread)
connectAll(thread, worker)
thread.start()

sys.exit(app.exec_())
