#include "AccelStepper.h"  
#include "MultiStepper.h"  

#include "Vector.h"
#include "Buffer.h"

Buffers<Angles> angles;

//***************************定义mega2560引脚*************************
//driver for the axis 1 驱动为轴1
//1轴(57步进电机) 
#define PUL1_PIN 13
#define DIR1_PIN 12
//driver for the axis 2
//2轴(57步进电机) 
#define PUL2_PIN 11
#define DIR2_PIN 10
//driver for the axis 3
//3轴(42步进电机)
#define PUL3_PIN 9
#define DIR3_PIN 8
//driver for the axis 4
//4轴(28步进电机)
#define PUL4_PIN 7
#define DIR4_PIN 6
//driver for the axis 5
//5轴(28步进电机)
#define PUL5_PIN 5
#define DIR5_PIN 4
//driver for the axis 6
//6轴(20步进电机)
#define PUL6_PIN 3
#define DIR6_PIN 2

//123轴使能引脚使用串联共用一个引脚 
#define EN321_PIN 14
//456轴使能引脚使用串联共用一个引脚 
#define EN456_PIN 15


//AccelStepper 设置步进模式 stepper1 对象名称 参一:步进模式 或者1 参二:电机PUL脉冲引脚 参三:电机DIR旋转方向引脚
AccelStepper stepper1(1, PUL1_PIN, DIR1_PIN);
AccelStepper stepper2(1, PUL2_PIN, DIR2_PIN);
AccelStepper stepper3(1, PUL3_PIN, DIR3_PIN);
AccelStepper stepper4(1, PUL4_PIN, DIR4_PIN);
AccelStepper stepper5(1, PUL5_PIN, DIR5_PIN);
AccelStepper stepper6(1, PUL6_PIN, DIR6_PIN);

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
  //***************** 定义mega2560步进电机引脚初始化参数 ***********************
  //321轴的使能使用同一个引脚
  pinMode(EN321_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(EN321_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平
  //456轴的使能使用同一个引脚
  pinMode(EN456_PIN, OUTPUT);  //使能引脚定义输出模式
  digitalWrite(EN456_PIN, LOW);  //关闭使能 低电平时可以驱动电机旋转 高电平不行 引脚如未定义则默认为低电平

  
  pinMode(PUL1_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR1_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL1_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR1_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL2_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR2_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL2_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR2_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL3_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR3_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL3_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR3_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL4_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR4_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL4_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR4_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL5_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR5_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL5_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR5_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

  pinMode(PUL6_PIN, OUTPUT);  //脉冲引脚定义输出模式
  pinMode(DIR6_PIN, OUTPUT);  //旋转方向引脚定义输出模式
  digitalWrite(PUL6_PIN, LOW); //一个高电平+一个低电平等一个脉冲
  digitalWrite(DIR6_PIN, HIGH); //LOW逆时针旋转 HIGH顺时针旋转

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



  //轴123细分:   [1]->8   [2]->8   [3]->8 
  //轴123减速比: [1]->24  [2]->24  [3]->24
  //轴123步距角: [1]->1.8 [2]->1.8 [3]->1.8

  //轴456细分:   [4]->8   [5]->8   [6]->128
  //轴456减速比: [4]->20  [5]->19  [6]->1
  //轴456步距角: [4]->1.8 [5]->1.8 [6]->1.8

  //Dt:                    Full Pulse(360°)
  //DtJ1 = 360/200/8/24;   38400
  //DtJ2 = 360/200/8/24;   38400
  //DtJ3 = 360/200/8/24;   38400
  //DtJ4 = 360/200/8/20;   32000
  //DtJ5 = 360/200/8/19;   30400
  //DtJ6 = 360/200/1/128;  25600
    
  //设置电机1/电机4/电机6初始状态为非0状态 免于使用负数参数 
  //例如:底盘一轴电机初始步进值为10000 moveTO(0)顺转90度 moveTO(10000)归位  moveTO(20000) 逆转90度
  //轴       脉冲范围         ROS范围  Studio<-->real
  //J1    0~12800~25600   -120~120      0      0    y = x/240*25600+12800
  //J2    0~6400~12800    -60~60        0      0    y = x/120*12800+6400
  //J3    0~6400~25600    -60~180       0      0    y = (x+60)/240*25600
  //J3'   0~19200~25600   -180~60       0      0    y = (-x+180)/240*25600
  //J4    0~16000~32000   -180~180      0      0    y = x/360*32000+16000
  //J5    0~10133~20266   -120~120      0      0    y = x/360*20266+10133
  //J6    0~12800~25600   -180~180      0      0    y = x/360*25600+12800


  //机械零位
  stepper1.setCurrentPosition(12800);  
  stepper2.setCurrentPosition(6400);  
  stepper3.setCurrentPosition(6400);  
  stepper4.setCurrentPosition(16000);
  stepper5.setCurrentPosition(10133);
  stepper6.setCurrentPosition(12800);

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
          stepper1.moveTo(12800);
          stepper2.moveTo(6400);
          stepper3.moveTo(6400);
          stepper4.moveTo(16000);
          stepper5.moveTo(10133);
          stepper6.moveTo(12800);

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

            uint32_t valSum = angle.a1%10 + angle.a2%10 + angle.a3%10 + angle.a4%10 + angle.a5%10 + angle.a6%10;
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
