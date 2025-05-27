import sys
from PyQt5.QtWidgets import QApplication, QLabel
import pyqt5_tools

print(pyqt5_tools.__file__)

app = QApplication(sys.argv)
label = QLabel("Hello from PyQt5!")
label.show()
sys.exit(app.exec_())
