
#- * - coding : utf - 8 - * -
import sys
import time
import six


import serial                                               #串口
from MsnhProto.MsnhProto import *                           #MsnhProto通信协议
from nanomsg import Socket, PUB, SUB, SUB_SUBSCRIBE, PUSH   #MQ通信
import threading                                            #多线程

                                                            #定义协议
from MsnhProto.py.RobotJoints import RobotJoints            #机器人关节
from MsnhProto.py.AD32 import AD32                          #AD信号0~(2^32-1)
from MsnhProto.py.DIO import DIO                            #IO信号ON-OFF
from MsnhProto.py.Base import Base                          #基础协议，用于解码


class RevThread(threading.Thread):#接收线程(从机器人)
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        while True:
            time.sleep(0.001)

class SendThread(threading.Thread):#发送线程(发送至机器人)
    def __init__(self):
        threading.Thread.__init__(self)
        self.bytes = []

    def run(self):
        while True:
            time.sleep(0.001)

if len(sys.argv) != 2:
    raise Exception("params error!")

protoPath = str(sys.argv[1])       #协议库路径从输入口传入
print("init protocal libs...",flush=True)
PyMsnhProtoInit.initLibPath(protoPath) #初始化MsnhProto库


th = RevThread()
th.start()
th1 = SendThread()
th1.start()
time.sleep(2)

sub = Socket(SUB)                               #接收(MsnhRobotStudio->机器人)
sub.connect('ipc:///tmp/Dummy-URDF_0_tf00')  	#接收地址
sub.set_string_option(SUB, SUB_SUBSCRIBE, b'')  #配置

pub = Socket(PUB)                               #广播(从机器人->MsnhRobotStudio)
pub.bind('ipc:///tmp/Dummy-URDF_0_joints')   	#广播地址

frame1 = RobotJoints(PyMsnhProto.DATA)          #定义一个机器人关节协议
frame1.setDataJoints([0,0,0,0,0,0])             #输入关节初始值
cmds = frame1.encode()                          #编码
pub.send(bytes(cmds))                           #发送至MsnhRobotStudio
print("sending zero point to MsnhRobotStudio ...",flush=True)


while True:#
    res = sub.recv()                            #接收从MsnhRobotStudio发送内容

    base = Base()                               #基础协议，用于解码
    ad32 = AD32()                               #AD协议
    dio  = DIO()                                #DIO协议
    robjps = RobotJoints()                      #机器人关节协议

    base.fromBytes(list(res))                   #初步解码，用于判断是哪种协议

    if base.getTypeID() == robjps.getID():      #解码是机器人关节协议ID
        robjps.fromBytes(list(res))             #使用机器人关节协议解码
        a = robjps.getDataJoints()              #获取从机器人发送过来的关节值
        print(a)								#机器人关节(可用将此参数发送至机器人)


    if base.getTypeID() == ad32.getID():       #解码是AD协议ID
        ad32.fromBytes(list(res))
        print(ad32.getDataName(),flush=True)   #AD变量名称 (可用将此参数发送至机器人,如舵机等)
        print(ad32.getDataAd(),flush=True)     #AD变量值   (可用将此参数发送至机器人,如舵机等)

    if base.getTypeID() == dio.getID():        #解码是IO协议ID
        dio.fromBytes(list(res))
        print(dio.getDataName(),flush=True)    #IO变量名称 (可用将此参数发送至机器人,如抓手,气泵等)
        print(dio.getDataIo(),flush=True)      #IO变量值   (可用将此参数发送至机器人,如抓手,气泵等)
sub.close()
pub.close()
