import sys
from datetime import datetime

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QLineEdit


class XLineEdit(QLineEdit):
    def __init__(self, parent=None):
        QLineEdit.__init__(self, parent)

    def keyPressEvent(self, ev):
        def appendText(text):
            today = datetime.today()
            day = today.strftime("%Y-%m-%d")
            hms = today.strftime("%Y-%m-%d %H:%M:%S")
            stamp = "".join(["[", hms, "]"])
            txt = " -> ".join([stamp, text])
            with open(day + ".txt", mode="a") as f:
                f.write(txt + "\n")

        if ev.key() not in [Qt.Key_Return, Qt.Key_Enter]:
            QLineEdit.keyPressEvent(self, ev)
            return
        appendText(self.text())
        self.setText("")


class App(QApplication):
    def __init__(self, argv):
        super(App, self).__init__(argv)
        self.line = XLineEdit()
        self.line.show()


if __name__ == "__main__":
    app = App(sys.argv)
    sys.exit(app.exec_())
