"""Fullscreen Sierpinski Triangle"""

import random

from PySide2.QtCore import QPoint, QTimer, Qt
from PySide2.QtGui import QColor, QPainter, QPen
from PySide2.QtWidgets import QApplication, QWidget


class SierpinskiTriangle(QWidget):
    """Fullscreen Sierpinski Triangle"""

    def __init__(self):
        """Ctor"""
        super().__init__()
        # do not clear previous drawing:
        self.setAttribute(Qt.WA_OpaquePaintEvent, True)

        self.start_points = None
        self.last_point = None
        self.color = self.get_color()
        self.showFullScreen()
        # reinit timer
        self.timer = QTimer()
        self.timer.timeout.connect(self.reinit)
        self.timer.setInterval(40_000)
        self.timer.start()

    def paintEvent(self, event):
        """Event"""
        rec = event.rect()
        painter = QPainter()
        painter.begin(self)
        if not self.start_points:
            painter.fillRect(rec, QColor(0, 0, 0, 100))
            self.update_start_points(rec)
        # make mypy happy
        assert isinstance(self.start_points, list)

        if not self.last_point:
            pt = random.choice(self.start_points)
        else:
            rp = random.choice(self.start_points)
            pt = QPoint(
                (self.last_point.x() + rp.x()) / 2,
                (self.last_point.y() + rp.y()) / 2,
            )

        self.last_point = pt
        painter.setPen(QPen(self.color, 0.8))
        painter.drawPoint(pt)
        painter.end()
        # start next update
        self.update()

    def update_start_points(self, rec):
        """Generate new start points"""
        rr = random.randrange
        yy = rr(3 * rec.height() // 4, rec.height())
        x0 = rr(0, rec.width() // 4)
        x1 = rec.width() // 2 + rr(0, rec.width() // 2)
        center = random.gauss(rec.width() // 2, 200)

        self.start_points = [
            QPoint(x0, yy),
            QPoint(x1, yy),
            QPoint(center, 0),
        ]

    def mouseReleaseEvent(self, event):
        """Reinit on click"""
        super().mouseReleaseEvent(event)
        self.reinit()

    def reinit(self):
        """Reinit new triangle"""
        self.start_points = None
        self.last_point = None
        self.color = self.get_color()

    def get_color(self):
        """Get random color"""
        return QColor.fromHsv(random.randrange(0, 360), 100, 255)


if __name__ == '__main__':
    app = QApplication()
    w = SierpinskiTriangle()
    w.show()
    app.exec_()
