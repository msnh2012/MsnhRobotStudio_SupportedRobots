
# -*- coding: utf-8 -*-
import sys
sys.path.append("..")
import time
from MsnhProto.MsnhProto import *
from MsnhProto.MsnhProtoDef import *
from nanomsg import Socket, PUB, SUB, SUB_SUBSCRIBE, PUSH
from nanomsg import Socket, PUB, SUB, SUB_SUBSCRIBE, PUSH
import threading
import util
import socket
import struct

protoPath = str(sys.argv[1])       #协议库路径从输入口传入
print("init protocal libs...",flush=True)
PyMsnhProtoInit.initLibPath(protoPath) #初始化MsnhProto库

HOST = "192.168.0.103"
PORT = 30003
tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket.connect((HOST, PORT))

tcp_socket1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_socket1.connect((HOST, PORT))

sub = Socket(SUB)
sub.connect('ipc:///tmp/UniversalRobots_UR5_0_tf00')
sub.set_string_option(SUB, SUB_SUBSCRIBE, b'')
frame = PyMsnhProto(RobotJoints.Info.typeID, type=PyMsnhProto.DATA) #定义


pub = Socket(PUB)
pub.bind('ipc:///tmp/UniversalRobots_UR5_0_joints')


class RevThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        while True:
            data = tcp_socket.recv(1116)
            if data.__len__() != 1116:
                continue
            position = struct.unpack('!6d', data[252:300]) #444:492
            position = np.asarray(position)
            position = position / 3.14159265453 * 180
            
            frame1 = PyMsnhProto(RobotJoints.Info.typeID, type=PyMsnhProto.DATA)
            frame1.setF32Vec(RobotJoints.Data.jointsF32Vec, position)
            cmds = frame1.serialize()
            pub.send(bytes(cmds))
            ##print(position)
            
class SendThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        while True:
            res = sub.recv()                                                #从studio读取关节信息
            frame.fromBytes(list(res))                                      #反序列化
            j = frame.getF32Vec(RobotJoints.Data.jointsF32Vec)              #获取实际关节值 
            j = np.asarray(j)
            j = j/180.0*3.141592654
            tcp_command = "servoj([%f,%f,%f,%f,%f,%f],0,0,0.008,0.03,300)\n" % (
            j[0], j[1], j[2], j[3], j[4], j[5])
            tcp_socket1.send(str.encode(tcp_command))
            tcp_socket1.send(str.encode(tcp_command))
            tcp_socket1.send(str.encode(tcp_command))
            tcp_socket1.send(str.encode(tcp_command))
            tcp_socket1.send(str.encode(tcp_command))
            time.sleep(0.008)

            
th = RevThread()
th.start()
th1 = SendThread()
th1.start()
