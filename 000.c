#include<reg52.h>
#include "intrins.h"
#include "codetab.h"
#include "LQ12864.h"

#define uchar unsigned char
#define uint  unsigned int
typedef unsigned char u8;


#define data  P0

/*
  数码管的显示原理就是通过控制数码管上每一块独立led的高低电平来让它显示字符的
  每一小块led的编号依次为：
          a
       f     b
          g
       e     c
          d    dp(小数点)
  编号顺序为：
  abcdefgdp
  0表示亮，1表示不亮
  所以数字0的对应编码就是:
  abcdefgdp
  00000011
  要显示出来必须转成对应的十六进制编码：0x03
*/
u8 code smgduan[10] = {
  0x03,  // 0  
  0x9F,  // 1
  0x25,  // 2 
  0x0D,  // 3
  0x99,  // 4
  0x49,  // 5
  0x41,  // 6
  0x1F,  // 7
  0x01,  // 8
  0x09   // 9
  }; // 显示0-9

uchar r_buf = 0;  // 储存单片机接收到的蓝牙信号
uchar xun_ji = 0;  // 定义进入循迹模式的变量
uchar bi_zhang = 0;  // 定义进入避障模式的变量
uchar temp = 10;  // 定义一个存储变量来控制OLED屏的显示
uint led = 10;  // 定义一个缓存来存储需要在数码管上显示的字符的下标

// 6个I/O口引脚
sbit Left_moto_pwm = P1 ^ 6;  // ENA接P1^6
sbit Right_moto_pwm = P1 ^ 7;  // ENB接P1^7
sbit p1 = P1 ^ 0;
sbit p2 = P1 ^ 1;
sbit p3 = P1 ^ 2;
sbit p4 = P1 ^ 3;

sbit Left_bizhang = P3 ^ 4;  // 左避障传感器I/O接口
sbit Right_bizhang = P3 ^ 5;  // 右避障传感器I/O接口

sbit leftled = P3 ^ 7;  // 左循迹传感器I/O接口
sbit rightled = P3 ^ 6;  // 右循迹传感器I/O接口

sbit beep = P2 ^ 3; // P2^3为单片机蜂鸣器的引脚

// 小车电机模式转换
void left_go()  // 左电机正转
{
  p1 = 0;
  p2 = 1;
}

void left_back()  // 左电机反转
{
  p1 = 1;
  p2 = 0;
}

void left_stop()  // 左电机停转
{
  p1 = 1;
  p2 = 1;
}

void right_go()  // 右电机正转
{
  p3 = 1;
  p4 = 0;
}

void right_back()  // 右电机反转
{
  p3 = 0;
  p4 = 1;
}

void right_stop()  // 右电机停转
{
  p3 = 1;
  p4 = 1;
}

bit Left_moto_stop = 1;
bit Right_moto_stop = 1;
extern unsigned char pwm_val_left = 0;
unsigned char push_val_left = 0;
extern unsigned char pwm_val_right = 0;
unsigned char push_val_right = 0;

void pwm_out_left_moto(void)     // 左电机调速
{
  if (Left_moto_stop)
  {
    if (pwm_val_left <= push_val_left)
      Left_moto_pwm = 1;
    else
      Left_moto_pwm = 0;
    if (pwm_val_left >= 10)
      pwm_val_left = 0;
  }
  else
    Left_moto_pwm = 0;
}

void pwm_out_right_moto(void)   // 右电机调速
{
  if (Right_moto_stop)
  {
    if (pwm_val_right <= push_val_right)
      Right_moto_pwm = 1;
    else
      Right_moto_pwm = 0;
    if (pwm_val_right >= 10)
      pwm_val_right = 0;
  }
  else
    Right_moto_pwm = 0;
}

uint Left_Speed_Ratio;  // 左电机调速的设定值
uint Right_Speed_Ratio;  // 右电机调速的设定值
unsigned int time = 0;

// 小车停止(左右电机停转)
void stop()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_stop();
  right_stop();
}

// 小车直走(左右电机正转)
void run()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_go();
  right_go();
}

// 小车左转(左电机停转，右电机正转)
void left()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_stop();
  right_go();
}

// 小车右转(左电机正转，右电机停转)
void right()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_go();
  right_stop();
}

// 小车后退(左右电机反转)
void back()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_back();
  right_back();
}

// 小车左转(左电机反转，右电机正转)
void left2()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_back();
  right_go();
}

// 小车右转(左电机正转，右电机反转)
void right2()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_go();
  right_back();
}

// 相关定时器函数的编写
void Timer0Init()   // 定时器初始化函数
{
  TMOD |= 0X01;  // 选择为定时器0模式，工作方式1，仅用TR0打开启动

  TH0 = 0XFC;  // 给定时器赋初值，定时1ms
  TL0 = 0X18;
  ET0 = 1;  // 打开定时器0中断允许
  EA = 1;  // 打开总中断
  TR0 = 1;  // 打开定时器
}

void timer0()interrupt 1 using 2  // 定时器中断函数，此处配置为1ms产生一次中断，对PWM的输出进行控制
{
  TH0 = 0XFC;  // 给定时器赋初值，定时1ms
  TL0 = 0X18;
  time++;
  pwm_val_left++;
  pwm_val_right++;
  pwm_out_left_moto();
  pwm_out_right_moto();
}

// 蓝牙模块初始化
void UART_INIT()
{
  SM0 = 0;
  SM1 = 1;
  REN = 1;
  EA = 1;
  ES = 1;
  TMOD = 0x20;
  TH1 = 0xfd;
  TL1 = 0xfd;
  TR1 = 1;
}

void xunji()  // 进入循迹模式
{
  // 1表示检测到黑线
  if (leftled == 1 && rightled == 0)  // 左循迹传感器检测到黑线，则向左转
  {
    Left_Speed_Ratio = 4;
    Right_Speed_Ratio = 6;
    stop();
    OLED_delay(2);
    left2();
    OLED_delay(10);
  }

  else if (leftled == 0 && rightled == 1)  // 右循迹传感器检测到黑线，则向右转
  {
    Left_Speed_Ratio = 6;
    Right_Speed_Ratio = 4;
    stop();
    OLED_delay(2);
    right2();
    OLED_delay(10);
  }

  else
  {
    Left_Speed_Ratio = 2;
    Right_Speed_Ratio = 2;
    stop();
    OLED_delay(2);
    run();
    OLED_delay(10);
  }
}

void bizhang()  // 进入避障模式
{
  // 0表示碰到障碍物
  if (Left_bizhang == 1 && Right_bizhang == 1)
  {
    beep = 1;
    Left_Speed_Ratio = 2;
    Right_Speed_Ratio = 2;
    run();
    OLED_delay(10);
  }

  else if (Left_bizhang == 1 && Right_bizhang == 0)
  {
    beep = 0;  // 右避障传感器检测到障碍物则蜂鸣器发声
    OLED_delay(2);  // 设置延迟让蜂鸣器发出的声音更响
    Left_Speed_Ratio = 4;
    Right_Speed_Ratio = 6;
    left2();
  }

  else   if (Left_bizhang == 0 && Right_bizhang == 1)
  {
    beep = 0;  // 左避障传感器检测到障碍物则蜂鸣器发声
    OLED_delay(2);  // 设置延迟让蜂鸣器发出的声音更响
    Left_Speed_Ratio = 6;
    Right_Speed_Ratio = 4;
    right2();
  }

  else if (Left_bizhang == 0 && Right_bizhang == 0)
  {
    beep = 0;  // 左右避障传感器检测到障碍物则蜂鸣器发声
    OLED_delay(2);  // 设置延迟让蜂鸣器发出的声音更响
    OLED_delay(10);
    Left_Speed_Ratio = 2;
    Right_Speed_Ratio = 2;
    back();
  }
}

// 主函数
void main(void)
{
  OLED_Init();
  UART_INIT();
  Timer0Init();
  Left_Speed_Ratio = 5;  // 初始化小车的速度为全速的50%
  Right_Speed_Ratio = 7; // 由于右电机比左电机的转速慢，所以右电机的速度要调高一些
  OLED_P32x32Ch(8, 0, 11);  
  OLED_P32x32Ch(88, 0, 12);
  while (1) 
  {    
    if(!r_buf)
    {
      data = smgduan[0];
    }
    
    if (r_buf == '5')  // 加速(调节速度为全速的50%)
    {
      Left_Speed_Ratio = 5;
      Right_Speed_Ratio = 7;
    }

    if (r_buf == '6') // 减速(调节速度为全速的20%)
    {
      Left_Speed_Ratio = 2;
      Right_Speed_Ratio = 4;
    }

    if (r_buf == 'a')  // 向前
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 0);  // 显示“向”
		    OLED_P32x32Ch(64, 4, 1);	// 显示“前”
      } 
      data = smgduan[Left_Speed_Ratio];
      run();
    }

    if (r_buf == '2')  // 向后
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 0);  // 显示“向”
		    OLED_P32x32Ch(64, 4, 13);  // 显示“后”
      }
      data = smgduan[Left_Speed_Ratio];
      back();
    }

    if (r_buf == '3')  // 向左转
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 0);  // 显示“向”
		    OLED_P32x32Ch(64, 4, 2);  // 显示“左”
      }
      data = smgduan[Left_Speed_Ratio];
      left();
    }

    if (r_buf == '4')  // 向右转
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 0);  // 显示“向”
		    OLED_P32x32Ch(64, 4, 3);  // 显示“右”
      }
      data = smgduan[Left_Speed_Ratio];
      right();
    }

    if (r_buf == '0')  // 停止
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 6);  // 显示“ ”，空格
		    OLED_P32x32Ch(64, 4, 6);  // 显示“ ”，空格
      }
      stop();
      OLED_delay(10);
      data = smgduan[0];
    }

    if (r_buf == '7')  // 判断是否进入巡线模式
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 7);  // 显示“巡”
        OLED_P32x32Ch(64, 4, 8);	// 显示“线”
      }
      OLED_delay(10);
      xun_ji = 1;
    }

    while (xun_ji)  // 进入巡线模式
    {
      if (r_buf == 'r') {
        data = smgduan[0];
        xun_ji = 0;
        break;
      }
      data = smgduan[2];
      xunji();
    }

    if (r_buf == '8')  // 判断是否进入避障模式
    {
      if (r_buf != temp) {
        temp = r_buf;
        OLED_P32x32Ch(32, 4, 9);  // 显示“避”
		    OLED_P32x32Ch(64, 4, 10);  // 显示“障”
      }
      OLED_delay(10);
      bi_zhang = 1;
    }

    while (bi_zhang)  // 进入避障模式 
    {
      if (r_buf == 'r')  // 重置退出避障模式
      {
        beep = 1;
        data = smgduan[0];
        bi_zhang = 0;
        break;
      }
      beep = 1;  // 初始化蜂鸣器不响
      data = smgduan[2];
      bizhang();
    }
    
  }
}

// 串口中断函数，接收手机发送的信号
void UARTInterrupt(void) interrupt 4
{
  if (RI)
  {
    RI = 0;
    r_buf = SBUF;
  }
}


