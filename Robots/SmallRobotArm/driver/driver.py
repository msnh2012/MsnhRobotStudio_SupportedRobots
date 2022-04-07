
# -*- coding: utf-8 -*-
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

def calSteps(angles):
	try:
		out = []
		out.append(int(angles[0]/240*20480+10240))
		out.append(int((angles[1]+78.5)/198.5*14115))
		out.append(int((-angles[2]+73.9)/253.9*22569))
		out.append(int(-angles[3]/360*17920+8960))
		out.append(int((angles[4]+45)/270*10080))
		out.append(int(-angles[5]/360*17920+8960))
		#print(out)
		return out
	except BaseException:
		return []

class RevThread(threading.Thread):#接收线程(从机器人)         
	def __init__(self):
		threading.Thread.__init__(self)

	def run(self):
		while True:
			global sendType
			time.sleep(0.001)
			str1 = ser.read_all()
			if len(str1) > 0:
				val = six.byte2int(str1)
				if val == 0x11 :
					sendType = 0
				elif val == 0x22 :
					sendType = 1
				elif val == 0x33 :
					sendType = 2

print("init serial port ...",flush=True)
ser=serial.Serial("COM3",115200,timeout=0.5)

class SendThread(threading.Thread):#发送线程(发送至机器人)   
	def __init__(self):
		threading.Thread.__init__(self)
		self.bytes = []

	def run(self):
		while True:
			global sendType
			time.sleep(0.001)
			if sendType == 0 and sendList.__len__()>0 :
				self.bytes = sendList.pop(0)
				buf = bufferList.pop(0)
				frame1.setDataJoints(buf)
				cmds = frame1.encode()
				pub.send(bytes(cmds))
				ser.write(self.bytes)
				ser.flush()
			elif sendType == 1:
				ser.write(self.bytes)
				print("resend")
			elif sendType == 2:
				print("full")

sendList = []
bufferList = []
sendType = 0 #0 normal, 1 resend, 2 nosend
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
ser.write([0xff,0xfe]) #reset
ser.flush()

sub = Socket(SUB)                               #接收(MsnhRobotStudio->机器人)
sub.connect('ipc:///tmp/SmallRobotArm_0_tf00')  #接收地址
sub.set_string_option(SUB, SUB_SUBSCRIBE, b'')  #配置
    
pub = Socket(PUB)                               #广播(从机器人->MsnhRobotStudio)
pub.bind('ipc:///tmp/SmallRobotArm_0_joints')   #广播地址

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
		steps = calSteps(a)                     #计算成步进电机脉冲
	
		if steps.__len__() == 0 :
			continue

		byte = []
		byte.append(0xff)                       #帧头
		byte.append(0xf0)

		sum = int(0)
		for n in steps:
			b = n.to_bytes(length=2,byteorder='little',signed=False)#关节角转BYTES,小端
			byte.append(b[0])                   
			byte.append(b[1])                   
			sum = sum + n                       #checksum

		b1 = sum.to_bytes(length=4,byteorder='little',signed=False)#checksum转BYTES,小端
		byte.append(b1[0])
		byte.append(b1[1])
		byte.append(b1[2])
		byte.append(b1[3])
		
		sendList.append(byte)
		bufferList.append(a)
  
	if base.getTypeID() == ad32.getID():       #解码是AD协议ID
		ad32.fromBytes(list(res))
		print(ad32.getDataName(),flush=True)   #AD变量名称
		print(ad32.getDataAd(),flush=True)     #AD变量值
  
	if base.getTypeID() == dio.getID():        #解码是IO协议ID
		dio.fromBytes(list(res))
		print(dio.getDataName(),flush=True)    #IO变量名称
		print(dio.getDataIo(),flush=True)      #IO变量值
sub.close()
pub.close()
