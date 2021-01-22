# coding:utf-8
import PyQt5 as qt
from PyQt5 import QtCore,QtGui,QtWidgets
import sys
import qtawesome
from PyQt5.QtWidgets import *
# from PyQt5.QtMultimedia import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
# from PyQt5.QtMultimediaWidgets import QVideoWidget
# from PyQt5.QtMultimedia import *
from PyQt5.QtCore import QTimer
# import time
import os
import pymysql
import subprocess
from queue import Queue
ipOfSniff = 'unknown'
portOfSniff = 'unknown'
envOfSniff = 'Windows'
md5OfSniff = 'No'
packOfSniff = 'unknown'
choTheDevice = 'unknown'

class secWin_choSniffEnv(QWidget):
    def __init__(self):
        super(secWin_choSniffEnv, self).__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Select the sniffing environment')
        self.resize(300, 50)

        layout = QHBoxLayout()  # 水平布局

        self.btn1 = QRadioButton('Linux')
        self.btn1.toggled.connect(self.buttonState)  # 设置按钮的槽函数
        layout.addWidget(self.btn1)

        self.btn2 = QRadioButton('Windows')
        self.btn2.setChecked(True)  # 设置默认选中状态
        self.btn2.toggled.connect(self.buttonState)   # 设置按钮的槽函数
        layout.addWidget(self.btn2)

        self.setLayout(layout)  # 设置布局

    def buttonState(self):
        global envOfSniff
        radioButton = self.sender()  # 事件发送者，获得信号的发送者
        if radioButton.isChecked():  # 按钮是否被选中
            envOfSniff = radioButton.text()
            print("ENVIRONMENT >>>" + radioButton.text())
            
class ThiWin_needMD5(QWidget):
    def __init__(self):
        super(ThiWin_needMD5, self).__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('Select the sniffing environment')
        self.resize(300, 50)

        layout = QHBoxLayout()  # 水平布局

        self.btn1 = QRadioButton('Yes')
        self.btn1.toggled.connect(self.buttonState)  # 设置按钮的槽函数
        layout.addWidget(self.btn1)

        self.btn2 = QRadioButton('No')
        self.btn2.setChecked(True)  # 设置默认选中状态
        self.btn2.toggled.connect(self.buttonState)   # 设置按钮的槽函数
        layout.addWidget(self.btn2)

        self.setLayout(layout)  # 设置布局

    def buttonState(self):
        global md5OfSniff
        radioButton = self.sender()  # 事件发送者，获得信号的发送者
        if radioButton.isChecked():  # 按钮是否被选中
            envOfSniff = radioButton.text()
            print("NEED MD5 CODING>>>" + radioButton.text())
                
class EmittingStream(QtCore.QObject):  
        textWritten = QtCore.pyqtSignal(str)  
        def write(self, text):
            self.textWritten.emit(str(text))

class MainUi(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()
        self.init_ui()

    def input_IP(self):
        global ipOfSniff
        ipOfSniff, okPressed = QInputDialog.getText(self, "Sniff information input","The IP you want to sniff:", QLineEdit.Normal, "")
        print("IP >>>",ipOfSniff)

    # def hiddde(self):
    #     # self.input.setVisible(False)
    #     # print("1")
    #     pass

    def input_port(self):
        global portOfSniff
        portOfSniff, okPressed = QInputDialog.getText(self, "Sniff information input","The POrt you want to sniff:", QLineEdit.Normal, "")
        print("PORT >>>",portOfSniff)

    def addcourierinfo(self):
        os.system("python insertcourier.py")

    def ChoSniffEnv(self):
        global envOfSniff
        secWin.show()

    def showDevice(self):
        if(envOfSniff == 'Linux'):
            os.system("python3 showDevice.py %s" % (envOfSniff))
        elif (envOfSniff == 'Windows'):
            os.system("python showDevice.py %s" % (envOfSniff))
        

    def needMD5(self):
        thiWin.show()

    def startSniff(self):
        print("当前的环境为>>>",envOfSniff)
        print("嗅探目标IP为>>>",ipOfSniff)
        print("嗅探目标端口为>>>",portOfSniff)
        print("是否解码MD5编码的用户名和密码>>>",md5OfSniff)
        print("嗅探包类型为>>>",packOfSniff)
        if(envOfSniff == 'Linux'):
            os.system("python3 beginSniff.py %s %s %s %s %s %s" % (envOfSniff,ipOfSniff,portOfSniff,md5OfSniff,packOfSniff,choTheDevice))
        elif (envOfSniff == 'Windows'):
            os.system("python beginSniff.py %s %s %s %s %s %s" % (envOfSniff,ipOfSniff,portOfSniff,md5OfSniff,packOfSniff,choTheDevice))
        # os.system("python beginSniff.py %s %s %s %s %s %s" % (envOfSniff,ipOfSniff,portOfSniff,md5OfSniff,packOfSniff,choTheDevice))
        print("..................")

    def outputWritten(self, text):  
        cursor = self.textEdit.textCursor()  
        cursor.movePosition(QtGui.QTextCursor.End)  
        cursor.insertText(text)  
        self.textEdit.setTextCursor(cursor)  
        self.textEdit.ensureCursorVisible()
         
    def type_datapackage(self):
        global packOfSniff
        packOfSniff, okPressed = QInputDialog.getText(self, "Sniff information input","The data_packet you want to sniff:", QLineEdit.Normal, "")
        print("data_packet",packOfSniff)
        # print("okPressed is",okPressed)
        

    def save_data(self):
        os.system("python change.py")

    def chooseDevice(self):
        global choTheDevice
        choTheDevice, okPressed = QInputDialog.getText(self, "Sniff information input","The DEVICE you want to sniff:", QLineEdit.Normal, "")
        print("DEVICE Num >>>",choTheDevice)

    def Contactus(self):
        print("2")
      
    def handleDisplay(self, data):
        # self.input.setText(data)
        # print("1")
        pass

    def init_ui(self):
        self.frame = []  # 存图片
        self.detectFlag = False  # 检测flag
        self.cap = []
        self.timer_camera = QTimer()     #定义定时器

        self.setFixedSize(1024,768)
        self.main_widget = QtWidgets.QWidget()  # 创建窗口主部件
        self.main_layout = QtWidgets.QGridLayout()  # 创建主部件的网格布局
        self.main_widget.setLayout(self.main_layout)  # 设置窗口主部件布局为网格布局

        self.left_widget = QtWidgets.QWidget()  # 创建左侧部件
        self.left_widget.setObjectName('left_widget')
        self.left_layout = QtWidgets.QGridLayout()  # 创建左侧部件的网格布局层
        self.left_widget.setLayout(self.left_layout) # 设置左侧部件布局为网格

        self.right_widget = QtWidgets.QWidget() # 创建右侧部件
        self.right_widget.setObjectName('right_widget')
        self.right_layout = QtWidgets.QGridLayout()
        self.right_widget.setLayout(self.right_layout) # 设置右侧部件布局为网格

        self.main_layout.addWidget(self.left_widget,0,0,12,2) # 左侧部件在第0行第0列，占8行3列
        self.main_layout.addWidget(self.right_widget,0,2,12,10) # 右侧部件在第0行第3列，占8行9列
        self.setCentralWidget(self.main_widget) # 设置窗口主部件

        self.left_close = QtWidgets.QPushButton("") # 关闭按钮
        self.left_visit = QtWidgets.QPushButton("") # 空白按钮
        self.left_mini = QtWidgets.QPushButton("")  # 最小化按钮
        # self.left_maxi = QtWidgets.QPushButton("")  # 最da化按钮
        self.left_close.clicked.connect(self.close)
        self.left_visit.clicked.connect(self.showMaximized)
        # self.left_maxi.clicked.connect(self.showMaximized)
        self.left_mini.clicked.connect(self.showMinimized)
        self.left_button_1 = QtWidgets.QPushButton("嗅探环境选择")
        self.left_button_1.clicked.connect(self.ChoSniffEnv)
        self.left_button_1.setObjectName('left_button')
        self.left_button_2 = QtWidgets.QPushButton("输入目的IP")
        self.left_button_2.setObjectName('left_button')
        self.left_button_2.clicked.connect(self.input_IP)
        self.left_button_3 = QtWidgets.QPushButton("限定端口")
        self.left_button_3.setObjectName('left_button')
        self.left_button_3.clicked.connect(self.input_port)
        self.left_button_4 = QtWidgets.QPushButton("选择目的包类型")
        self.left_button_4.setObjectName('left_button')
        self.left_button_4.clicked.connect(self.type_datapackage)
        self.left_button_5 = QtWidgets.QPushButton("MD5解码")
        self.left_button_5.clicked.connect(self.needMD5)
        self.left_button_5.setObjectName('left_button')
        self.left_button_6 = QtWidgets.QPushButton("保存数据")
        self.left_button_6.setObjectName('left_button')
        self.left_button_6.clicked.connect(self.save_data)
        self.left_button_7 = QtWidgets.QPushButton("开始嗅探")
        self.left_button_7.clicked.connect(self.startSniff)
        self.left_button_7.setObjectName('left_button')
        # self.left_button_8 = QtWidgets.QPushButton("停止嗅探")
        # self.left_button_8.clicked.connect(self.addcourierinfo)
        # self.left_button_8.setObjectName('left_button')
        self.left_button_9 = QtWidgets.QPushButton("选择网卡")
        self.left_button_9.setObjectName('left_button')
        self.left_button_9.clicked.connect(self.chooseDevice)
        self.left_button_10 = QtWidgets.QPushButton(qtawesome.icon('fa.question',color='white'),"遇到问题")
        self.left_button_10.setObjectName('left_button')
        self.left_button_10.clicked.connect(self.Contactus)
        self.left_button_11 = QtWidgets.QPushButton("显示网卡")
        self.left_button_11.clicked.connect(self.showDevice)
        self.left_button_11.setObjectName('left_button')

        self.left_xxx = QtWidgets.QPushButton(" ")

        self.left_layout.addWidget(self.left_mini,0,0,1,3)
        self.left_layout.addWidget(self.left_visit,0,1,1,2)
        self.left_layout.addWidget(self.left_close,0,2,1,1)
        
        # self.left_layout.addWidget(self.left_label_1,1,0,1,3)
        self.left_layout.addWidget(self.left_button_1, 2,0,1,3)
        self.left_layout.addWidget(self.left_button_2, 3,0,1,3)
        self.left_layout.addWidget(self.left_button_3, 4,0,1,3)
        self.left_layout.addWidget(self.left_button_4, 6,0,1,3)
        self.left_layout.addWidget(self.left_button_5, 7,0,1,3)
        self.left_layout.addWidget(self.left_button_6, 8,0,1,3)
        self.left_layout.addWidget(self.left_button_7, 10,0,1,3)
        # self.left_layout.addWidget(self.left_button_8, 11,0,1,3)
        self.left_layout.addWidget(self.left_button_11, 12,0,1,3)
        self.left_layout.addWidget(self.left_button_9, 13,0,1,3)
        self.left_layout.addWidget(self.left_button_10, 14,0,1,3)

        self.right_bar_widget = QtWidgets.QWidget() # 右侧顶部搜索框部件
        self.right_bar_layout = QtWidgets.QGridLayout() # 右侧顶部搜索框网格布局
        self.right_bar_widget.setLayout(self.right_bar_layout)
        self.right_layout.addWidget(self.right_bar_widget, 0,0,0,0)#使用说明的文本换行自适应大小

        self.ins = QtWidgets.QLabel("哪个男孩不想拥有自己的嗅探器呢")#这里需要添加读取一个txt文件的内容并显示
        self.right_bar_layout.addWidget(self.ins,0,0,1,0)
        self.ins.setStyleSheet("color:rgb(94,94,94)")
        self.ins.setFont(QFont("MFLiHei_Noncommercial-Regular",20))
        self.ins.setAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignTop)

        self.segmentation = QtWidgets.QLabel("-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")#这里需要添加读取一个txt文件的内容并显示
        self.right_bar_layout.addWidget(self.segmentation,1,0,1,1)
        # self.segmentation.setStyleSheet("color:red")
        # self.segmentation.setFont(QFont("方正姚体 常规",12,63))
        self.segmentation.setAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignTop)
        self.segmentation.setStyleSheet("color:rgb(94,94,94)")
        self.segmentation.setFont(QFont("MFLiHei_Noncommercial-Regular",20))

        
        self.segmentation3 = QtWidgets.QLabel("使用tips：1>>选择嗅探环境   2>>显示网卡   3>>输入网卡   前三步必做")#这里需要添加读取一个txt文件的内容并显示
        self.right_bar_layout.addWidget(self.segmentation3,2,0,1,1)
        # self.segmentation.setStyleSheet("color:red")
        # self.segmentation.setFont(QFont("方正姚体 常规",12,63))
        self.segmentation3.setAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignTop)
        self.segmentation3.setStyleSheet("color:rgb(94,94,94)")
        self.segmentation3.setFont(QFont("MFLiHei_Noncommercial-Regular",10))
        self.segmentation4 = QtWidgets.QLabel("使用tips：其余各项可以按需输入>>>敏感包类型为:tcp==default(http\\ftp\\telnet)")#这里需要添加读取一个txt文件的内容并显示
        self.right_bar_layout.addWidget(self.segmentation4,3,0,1,1)
        # self.segmentation.setStyleSheet("color:red")
        # self.segmentation.setFont(QFont("方正姚体 常规",12,63))
        self.segmentation4.setAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignTop)
        self.segmentation4.setStyleSheet("color:rgb(94,94,94)")
        self.segmentation4.setFont(QFont("MFLiHei_Noncommercial-Regular",10))

        self.segmentation2 = QtWidgets.QLabel("-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")#这里需要添加读取一个txt文件的内容并显示
        self.right_bar_layout.addWidget(self.segmentation2,4,0,1,1)
        # self.segmentation.setStyleSheet("color:red")
        # self.segmentation.setFont(QFont("方正姚体 常规",12,63))
        self.segmentation2.setAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignTop)
        self.segmentation2.setStyleSheet("color:rgb(94,94,94)")
        self.segmentation2.setFont(QFont("MFLiHei_Noncommercial-Regular",20))

        self.scrollArea_2 = QtWidgets.QScrollArea()
        self.right_bar_layout.addWidget(self.scrollArea_2,5,0,1,1)
        # self.scrollArea_2.setGeometry(QtCore.QRect(10, 20, 731, 301))
        self.scrollArea_2.setWidgetResizable(True)
        self.scrollArea_2.setObjectName("scrollArea_2")
        self.scrollAreaWidgetContents_2 = QtWidgets.QWidget()
        self.scrollAreaWidgetContents_2.setGeometry(QtCore.QRect(0, 0, 729, 320))
        self.scrollAreaWidgetContents_2.setObjectName("scrollAreaWidgetContents_2")
        self.textEdit = QtWidgets.QTextEdit(self.scrollAreaWidgetContents_2)
        # self.textEdit.adjustSize()
        self.textEdit.setGeometry(QtCore.QRect(0, 0, 870, 650))
        self.textEdit.setObjectName("textEdit")
        self.textEdit.setReadOnly(True)
        self.scrollArea_2.setWidget(self.scrollAreaWidgetContents_2)
        self.textEdit.setReadOnly(True)


        sys.stdout = EmittingStream(textWritten=self.outputWritten)  
        sys.stderr = EmittingStream(textWritten=self.outputWritten)

        self.left_close.setFixedSize(15,15) # 设置关闭按钮的大小
        self.left_visit.setFixedSize(15,15)  # 设置按钮大小
        self.left_mini.setFixedSize(15,15) # 设置最小化按钮大小
        self.left_close.setStyleSheet('''QPushButton{background:#F76677;border-radius:5px;}QPushButton:hover{background:red;}''')
        self.left_visit.setStyleSheet('''QPushButton{background:#F7D674;border-radius:5px;}QPushButton:hover{background:yellow;}''')
        self.left_mini.setStyleSheet('''QPushButton{background:#6DDF6D;border-radius:5px;}QPushButton:hover{background:green;}''')
        self.left_widget.setStyleSheet('''
    QPushButton{border:none;color:white;}
    QPushButton#left_label{
        border:none;
        border-bottom:1px solid white;
        font-size:18px;
        font-weight:700;
        font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
    }
    QPushButton#left_button:hover{border-left:4px solid red;font-weight:700;}
    QWidget#left_widget{
    background:gray;
    border-top:1px solid white;
    border-bottom:1px solid white;
    border-left:1px solid white;
    border-top-left-radius:10px;
    border-bottom-left-radius:10px;
}
''')
        self.right_widget.setStyleSheet('''
    QWidget#right_widget{
        color:#232C51;
        background:white;
        border-top:1px solid darkGray;
        border-bottom:1px solid darkGray;
        border-right:1px solid darkGray;
        border-top-right-radius:10px;
        border-bottom-right-radius:10px;
    }
    # QLabel#right_lable{
    #     border:none;
    #     font-size:16px;
    #     font-weight:700;
    #     font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
    # }
    ins{
        background:red;
    }
''')
        # self.setWindowOpacity(0.9) # 设置窗口透明度
        self.setAttribute(QtCore.Qt.WA_TranslucentBackground) # 设置窗口背景透明
        self.setWindowFlag(QtCore.Qt.FramelessWindowHint) # 隐藏边框
        self.main_layout.setSpacing(0)
    def showDialog1(self):
        fname = 'C:/Users/49353/Desktop/cao.txt'
        if fname[0]:
            f = open(fname[0], 'r')
            with f:
                data = f.read()
                self.textEdit.setText(data)

if __name__ == '__main__':

    # main()
    app = QtWidgets.QApplication(sys.argv)
    gui = MainUi()
    secWin = secWin_choSniffEnv()
    thiWin = ThiWin_needMD5()
    gui.show()
    sys.exit(app.exec_())