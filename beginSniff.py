import sys
from queue import Queue
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import time
import subprocess
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
        # for i in range(1000):
        #     print (i)
        subp=subprocess.Popen(start,shell=True,stdout=subprocess.PIPE)
        c=subp.stdout.readline()
        while c:
            try:
                c = str(c,encoding='gb18030')          
            except:
                pass
            # c.replace("b'","")
            c = subp.stdout.readline()
            print (c.decode('gb18030'))
            # self.showDataWindow.setText(c)
            
        subp.wait()

# An Example application QWidget containing the textedit to redirect stdout to
class MainUi(QWidget):
    def __init__(self,*args,**kwargs):
        QWidget.__init__(self,*args,**kwargs)
        self.layout = QVBoxLayout(self)
        self.setMinimumHeight(500)  #窗体最小高度
        self.setMinimumWidth(1000) #窗体最小宽度
        self.textedit = QTextEdit()
        self.button = QPushButton('start sniff')
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
    ip = sys.argv[2]
    port = sys.argv[3]
    md5 = sys.argv[4]
    pack = sys.argv[5]
    flag = sys.argv[6]
    if(env == 'Windows'):
        if(ip == 'unknown' and port == 'unknown'):
            if(pack == 'unknown'):
                print('全局模式')
                start = 'MySniff.exe --eth=' + flag + ' --debug '
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('抓取敏感数据模式')
                start = 'MySniff.exe --eth=' + flag + ' -- ' + pack + ' --debug'
            else:
                print(pack+' 协议模式')
                start = 'MySniff.exe --eth=' + flag + ' --' + pack + ' --debug'
        elif(ip != 'unknown' and port == 'unknown'):
            if(pack == 'unknown'):
                print('IP全局模式')
                start = 'MySniff.exe --eth=' + flag + ' --debug ' + ' --ip ' + ip
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('IP抓取敏感数据模式')
                start = 'MySniff.exe --eth=' + flag + ' -- ' + pack + ' --debug' + ' --ip ' + ip
            else:
                print(pack+' AND IP协议模式')
                start = 'MySniff.exe --eth=' + flag + ' --' + pack + ' --debug' + ' --ip ' + ip
        elif(ip =='unknown' and port != 'unknown'):
            if(pack == 'unknown'):
                print('PORT全局模式')
                start = 'MySniff.exe --eth=' + flag + ' --debug ' + ' --port '+ port
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('PORT抓取敏感数据模式')
                start = 'MySniff.exe --eth=' + flag + ' -- ' + pack + ' --debug' + ' --port '+ port
            else:
                print(pack+' AND PORT 协议模式')
                start = 'MySniff.exe --eth=' + flag + ' --' + pack + ' --debug' + ' --port '+ port
        elif(ip != 'unknown' and port != 'unknown'):
            if(pack == 'unknown'):
                print('Win全局模式')
                start = 'MySniff.exe --eth=' + flag   + ' --port '+ port + ' --ip ' + ip
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('抓取敏感数据模式')
                start = 'MySniff.exe --eth=' + flag + ' -- ' + pack + ' --debug'  + ' --port '+ port + ' --ip ' + ip
            else:
                print(pack+' 协议模式')
                start = 'MySniff.exe --eth=' + flag + ' --' + pack + ' --debug'  + ' --port '+ port + ' --ip ' + ip
    elif(env == 'Linux'):
        if(ip == 'unknown' and port == 'unknown'):
            if(pack == 'unknown'):
                print('全局模式')
                start = './MySniff --eth=' + flag + ' --debug '
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('抓取敏感数据模式')
                start = './MySniff --eth=' + flag + ' -- ' + pack + ' --debug'
            else:
                print(pack+' 协议模式')
                start = './MySniff --eth=' + flag + ' --' + pack + ' --debug'
        elif(ip != 'unknown' and port == 'unknown'):
            if(pack == 'unknown'):
                print('IP全局模式')
                start = './MySniff --eth=' + flag + ' --debug ' + ' --ip ' + ip
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('IP抓取敏感数据模式')
                start = './MySniff --eth=' + flag + ' -- ' + pack + ' --debug' + ' --ip ' + ip
            else:
                print(pack+' AND IP协议模式')
                start = './MySniff --eth=' + flag + ' --' + pack + ' --debug' + ' --ip ' + ip
        elif(ip =='unknown' and port != 'unknown'):
            if(pack == 'unknown'):
                print('PORT全局模式')
                start = './MySniff --eth=' + flag + ' --debug ' + ' --port '+ port
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('PORT抓取敏感数据模式')
                start = './MySniff --eth=' + flag + ' -- ' + pack + ' --debug' + ' --port '+ port
            else:
                print(pack+' AND PORT 协议模式')
                start = './MySniff --eth=' + flag + ' --' + pack + ' --debug' + ' --port '+ port
        elif(ip != 'unknown' and port != 'unknown'):
            if(pack == 'unknown'):
                print('全局模式')
                start = './MySniff --eth=' + flag + ' --debug '  + ' --port '+ port + ' --ip ' + ip
            elif(pack == 'tcp=fault' or pack == 'tcp=http' or pack == 'tcp=ftp' or pack == 'tcp=telnet'):
                print('抓取敏感数据模式')
                start = './MySniff --eth=' + flag + ' -- ' + pack + ' --debug'  + ' --port '+ port + ' --ip ' + ip
            else:
                print(pack+' 协议模式')
                start = './MySniff --eth=' + flag + ' --' + pack + ' --debug'  + ' --port '+ port + ' --ip ' + ip
    print(start)
    # Create Queue and redirect sys.stdout to this queue
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
