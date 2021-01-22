#!/usr/bin/python3
#coding=utf-8
import sys
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import time
import subprocess
import threading
from queue import Queue
from queue import Empty
# import queue as Queue
# from queue import Empty
# The new Stream Object which replaces the default stream associated with sys.stdout
# This object just puts data in a queue!
start=''

class WriteStream(object):
    def __init__(self,queue):
        self.queue = queue

    def write(self, text):
        self.queue.put(text)

# A QObject (to be run in a QThread) which sits waiting for data to come through a Queue.Queue().
# It blocks until data is available, and one it has got something from the queue, it sends
# it to the "MainThread" by emitting a Qt Signal 
class MyReceiver(QObject):
    mysignal = pyqtSignal(str)

    def __init__(self,queue,*args,**kwargs):
        QObject.__init__(self,*args,**kwargs)
        self.queue = queue

    @pyqtSlot()
    def run(self):
        while True:
            text = self.queue.get()
            self.mysignal.emit(text)

# An example QObject (to be run in a QThread) which outputs information with print
class LongRunningThing(QObject):
    @pyqtSlot()
    def run(self):
        subp=subprocess.Popen(start,shell=True,stdout=subprocess.PIPE)
        c=subp.stdout.readline()
        while c:
            try:
                c = str(c,encoding='gb18030')          
            except:
                pass
            c = subp.stdout.readline()
            print (c.decode('gb18030'))           
        subp.wait()

# An Example application QWidget containing the textedit to redirect stdout to
class MainUi(QWidget):
    def __init__(self,*args,**kwargs):
        QWidget.__init__(self,*args,**kwargs)
        self.layout = QVBoxLayout(self)
        self.setMinimumHeight(500)  #窗体最小高度
        self.setMinimumWidth(1000) #窗体最小宽度
        self.textedit = QTextEdit()
        self.button = QPushButton('SHOW DEVICE')
        self.button.clicked.connect(self.start_thread)
        self.layout.addWidget(self.textedit)
        self.layout.addWidget(self.button)
        self.textedit.setReadOnly(True)
        # self.layout.setStretchFactor(self.textedit,4)
        # self.layout.setStretchFactor(self.button,2)
        
    @pyqtSlot()
    # def run(self):
    #     for i in range(1000):
    #         print(i)
    @pyqtSlot(str)
    def append_text(self,text):
        self.textedit.moveCursor(QTextCursor.End)
        self.textedit.insertPlainText( text )

    @pyqtSlot()
    def start_thread(self):
        self.thread = QThread()
        self.long_running_thing = LongRunningThing()
        self.long_running_thing.moveToThread(self.thread)
        self.thread.started.connect(self.long_running_thing.run)
        self.thread.start()

def startSniff():
    global start
    env = sys.argv[1]
    # Create Queue and redirect sys.stdout to this queue
    if(env == 'Windows'):
        start = 'MySniff.exe --eth'
    elif(env == 'Linux'):
        start = './MySniff --eth'
    queue = Queue()
    sys.stdout = WriteStream(queue)

    # Create QApplication and QWidget
    qapp = QApplication(sys.argv)  
    app = MainUi()
    app.show()

    # Create thread that will listen on the other end of the queue, and send the text to the textedit in our application
    thread = QThread()
    my_receiver = MyReceiver(queue)
    my_receiver.mysignal.connect(app.append_text)
    my_receiver.moveToThread(thread)
    thread.started.connect(my_receiver.run)
    thread.start()

    qapp.exec_()

if __name__ == '__main__':
    startSniff()
