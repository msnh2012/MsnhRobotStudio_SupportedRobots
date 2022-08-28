#include "AccelStepper.h"  
#include "MultiStepper.h"  

#include "Vector.h"
#include "Buffer.h"

Buffers<Angles> angles;

//***************************定义mega2560引脚*************************
//driver for the axis 1 驱动为轴1
//1轴(57步进电机) TB6600驱动器 STEP脉冲连接mega2560的39引脚(由下往上第8引脚) DIR方向连接37引脚(由下往上第9引脚)
#define PUL1_PIN 39
#define DIR1_PIN 37
//driver for the axis 2
//2轴(57步进电机) TB6600驱动器 STEP脉冲连接mega2560的43引脚(由下往上第6引脚) DIR方向连接41引脚(由下往上第7引脚)
#define PUL2_PIN 43
#define DIR2_PIN 41
//driver for the axis 3
//3轴(42步进电机) TB6600驱动器 STEP脉冲连接mega2560的47引脚(由下往上第4引脚) DIR方向连接45引脚(由下往上第5引脚)
#define PUL3_PIN 47
#define DIR3_PIN 45
//driver for the axis 4
//4轴(28步进电机) DRV8825驱动器(Z区引脚) STEP脉冲连接mega2560的46引脚 DIR方向连接48引脚
#define PUL4_PIN 46
#define DIR4_PIN 48
//driver for the axis 5
//5轴(28步进电机) DRV8825驱动器(Y区引脚) STEP脉冲连接mega2560的A6引脚 DIR方向连接A7引脚
#define PUL5_PIN A6
#define DIR5_PIN A7
//driver for the axis 6
//6轴(20步进电机) DRV8825驱动器(X区引脚) STEP脉冲连接mega2560的A0引脚 DIR方向连接A1引脚
#define PUL6_PIN A0
#define DIR6_PIN A1

//123轴使能引脚使用串联共用一个引脚 连接 #define EN321_PIN 32 (最右侧由下往上第3引脚)
#define EN321_PIN 32
//4轴使能引脚 连接 #define EN4_PIN A8 (DRV8825驱动器上不需要接线)
#define EN4_PIN A8
//5轴使能引脚 连接 #define EN5_PIN A2 (DRV8825驱动器上不需要接线)
#define EN5_PIN A2
//6轴使能引脚 连接 #define EN6_PIN 38 (DRV8825驱动器上不需要接线)
#define EN6_PIN 38

// ********************* 定义步进电机对象 *************************
// 电机步进方式定义 参数未使用
#define FULLSTEP 4 //全步进参数[全速前进]
#define HALFSTEP 8 //半步进参数[半步]

//AccelStepper 设置步进模式 stepper1 对象名称 参一:步进模式 或者1 参二:电机PUL脉冲引脚 参三:电机DIR旋转方向引脚
AccelStepper stepper1(1, 39, 37);
AccelStepper stepper2(1, 43, 41);
AccelStepper stepper3(1, 47, 45);
AccelStepper stepper4(1, 46, 48);
AccelStepper stepper5(1, A6, A7);
AccelStepper stepper6(1, A0, A1);

int mark = 0;
String comdata = "";
bool start = false;

bool readyToread = false;
uint8_t dataState = 0; // 0-idle 1-angle

std::Vector<uint8_t> datas;

int frameCnt = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(11, OUTPUT); //设置11号端口作为输出端口 可用于led或电磁铁
  digitalWrite(11, LOW); //电磁铁先不通电

  //***************** 定义mega2560步进电机引脚初始化参数 ***********************
  //321轴的使能使用同一个引脚
  pinMode(EN321_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(EN321_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平

  pinMode(PUL1_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR1_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL1_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR1_PIN, LOW); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL2_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR2_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL2_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR2_PIN, LOW); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL3_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR3_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL3_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR3_PIN, LOW); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL4_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR4_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  pinMode(EN4_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(PUL4_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR4_PIN, LOW); //LOW逆时针旋转 HIGH顺时针旋转
  digitalWrite(EN4_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平

  pinMode(PUL5_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR5_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  pinMode(EN5_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(PUL5_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR5_PIN, LOW); //LOW逆时针旋转 HIGH顺时针旋转
  digitalWrite(EN5_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平

  pinMode(PUL6_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR6_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  pinMode(EN6_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(PUL6_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR6_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转
  digitalWrite(EN6_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平

  ////补充stepper1.setSpeed()是电机匀速旋转 配合runspeed函数使用
  stepper1.setMaxSpeed(10000.0);  //电机最大速度 非匀速 配合run函数使用 最大值10000
  stepper1.setAcceleration(30000.0);  //电机加速度 非匀速 配合run函数使用 最大值10000

  stepper2.setMaxSpeed(10000.0); //推荐10000
  stepper2.setAcceleration(10000.0); //推荐10000

  stepper3.setMaxSpeed(10000.0); //推荐10000
  stepper3.setAcceleration(10000.0); //推荐10000

  stepper4.setMaxSpeed(10000.0); //推荐10000
  stepper4.setAcceleration(10000.0); //推荐10000

  stepper5.setMaxSpeed(10000.0); //推荐10000
  stepper5.setAcceleration(10000.0); //推荐10000

  stepper6.setMaxSpeed(10000.0); //推荐10000
  stepper6.setAcceleration(10000.0); //推荐10000

  //设置电机1/电机4/电机6初始状态为非0状态 免于使用负数参数 
  //例如:底盘一轴电机初始步进值为10000 moveTO(0)顺转90度 moveTO(10000)归位  moveTO(20000) 逆转90度
  
  //轴    脉冲范围(极小l~零位o~极大m)    ROS范围(Rl~Rm)      Studio<-->real      脉冲与转角对应公式             细分a   步距角b  传动比c   极大
  //J1    0~10240~20480              -120~120               0      0        y = x/240*20480+10240          32     1.8    4.8       m = 360/b*a*c/360*(abs(Rm-Rl))
  //J2    0~5582~14115               -78.5~120              0      -90      y = (x+78.5)/198.5*14115       32     1.8    4.0       m = 360/b*a*c/360*(abs(Rm-Rl)) 
  //J3    0~6569~22569               73.9~-180              0      90       y = (-x+73.9)/253.9*22569      32     1.8    5.0       m = 360/b*a*c/360*(abs(Rm-Rl))
  //J4    0~8960~17920               -180~180               0      0        y = x/360*17920+8960           32     1.8    2.8       m = 360/b*a*c/360*(abs(Rm-Rl))
  //J5    0~8400~10080               -45~225                0      -90      y = (x+45)/270*10080           32     1.8    2.1       m = 360/b*a*c/360*(abs(Rm-Rl))
  //J6    0~3200~6400                -180~180               0      0        y = x/360*6400+3200            32     1.8    1.0       m = 360/b*a*c/360*(abs(Rm-Rl))

 
  //机械零位
  stepper1.setCurrentPosition(10240);  
  stepper4.setCurrentPosition(8960);
  stepper5.setCurrentPosition(5040);
  stepper6.setCurrentPosition(3200);

}

void loop()
{
if (!stepper1.isRunning() && !stepper2.isRunning() && !stepper3.isRunning() &&
        !stepper4.isRunning() && !stepper5.isRunning() && !stepper6.isRunning())
    {
      if (!angles.isEmpty())
      {
        Angles ang;
        angles.getFront(ang);
        stepper1.moveTo(ang.a1);
        stepper2.moveTo(ang.a2);
        stepper3.moveTo(ang.a3);
        stepper4.moveTo(ang.a4);
        stepper5.moveTo(ang.a5);
        stepper6.moveTo(ang.a6);

        if (!angles.isFull())
        {
          Serial.write(0x11);
        }
      }
    }

    stepper1.run();  //驱动步进电机按照指定参数旋转
    stepper2.run();
    stepper3.run();
    stepper4.run();
    stepper5.run();
    stepper6.run();
}

void serialEvent() 
{
  while (Serial.available() > 0)
    {
      uint8_t ch = uint8_t(Serial.read());

      if (readyToread)
      {
        datas.push_back(ch);

        if (datas.size() == 2 && datas[1] == 0xfe)
        {
          //MsnhRobotStudio零位
          stepper1.moveTo(10240);
          stepper2.moveTo(5582);
          stepper3.moveTo(6569);
          stepper4.moveTo(8960);
          stepper5.moveTo(1680);
          stepper6.moveTo(3200);

          datas.clear();
          readyToread = false;
          continue;
        }
        if (datas.size() == 18)
        {
          if (datas[1] == 0xf0)
          {
            UInt16 val;
            Angles angle;
            val.bytes[0] = (uint8_t)datas[2];
            val.bytes[1] = (uint8_t)datas[3];
            angle.a1 = val.val;
            val.bytes[0] = (uint8_t)datas[4];
            val.bytes[1] = (uint8_t)datas[5];
            angle.a2 = val.val;
            val.bytes[0] = (uint8_t)datas[6];
            val.bytes[1] = (uint8_t)datas[7];
            angle.a3 = val.val;
            val.bytes[0] = (uint8_t)datas[8];
            val.bytes[1] = (uint8_t)datas[9];
            angle.a4 = val.val;
            val.bytes[0] = (uint8_t)datas[10];
            val.bytes[1] = (uint8_t)datas[11];
            angle.a5 = val.val;
            val.bytes[0] = (uint8_t)datas[12];
            val.bytes[1] = (uint8_t)datas[13];
            angle.a6 = val.val;

            UInt32 sum;
            sum.bytes[0] = (uint8_t)datas[14];
            sum.bytes[1] = (uint8_t)datas[15];
            sum.bytes[2] = (uint8_t)datas[16];
            sum.bytes[3] = (uint8_t)datas[17];

            uint32_t valSum = angle.a1 + angle.a2 + angle.a3 + angle.a4 + angle.a5 + angle.a6;
            if (sum.val == valSum)
            {
              angles.addOne(angle);

              if (angles.isFull())
              {
                Serial.write(0x33);
              }
              else
              {
                Serial.write(0x11);
              }
            }
            else
            {
              Serial.flush();
              Serial.write(0x22);
            }

            datas.clear();
            readyToread = false;
            continue;
          }
          else
          {
            datas.clear();
            readyToread = false;
            continue;
          }
        }
      }
      else if (!readyToread && ch == 0xff)
      {
        datas.push_back(ch);
        readyToread = true;
      }
    }
}
