/*
#include "intrins.h"
#include "codetab.h"
#include "LQ12864.h"

void main(void)
{
	OLED_Init();
	while(1)
	{
		OLED_P16x16Ch(56, 4, 32);//Ç°
		OLED_delay(500);
		OLED_CLS();//ºÚÆÁ
		OLED_P16x16Ch(56, 4, 33);//ºó
		OLED_delay(500);
		OLED_CLS();//ºÚÆÁ
		OLED_P16x16Ch(56, 4, 34);//×ó
		OLED_delay(500);
		OLED_CLS();//ºÚÆÁ
		OLED_P16x16Ch(56, 4, 35);//ÓÒ
		OLED_delay(500);
		OLED_CLS();//ºÚÆÁ
	}
}
*/

//#include<reg52.h>
#include "intrins.h"
#include "codetab.h"
#include "LQ12864.h"

#define uchar unsigned char
#define uint  unsigned int
uchar r_buf = 0;
uchar xun_ji = 0;
uchar bi_zhang = 0;
uchar temp = 10;

// 6?I/O?????
sbit Left_moto_pwm = P1 ^ 6;  // ENA?P1^6
sbit Right_moto_pwm = P1 ^ 7;  // ENB?P1^7
sbit p1 = P1 ^ 0;
sbit p2 = P1 ^ 1;
sbit p3 = P1 ^ 2;
sbit p4 = P1 ^ 3;

sbit Left_bizhang = P3 ^ 4; // ???
sbit Right_bizhang = P3 ^ 5; // ???

sbit leftled = P3 ^ 6; // ???
sbit rightled = P3 ^ 7; // ???

// ????????
void left_go() // ?????
{
  p1 = 0;
  p2 = 1;
}

void left_back() // ?????
{
  p1 = 1;
  p2 = 0;
}

void left_stop() // ?????
{
  p1 = 1;
  p2 = 1;
}

void right_go() // ?????
{
  p3 = 1;
  p4 = 0;
}

void right_back() // ?????
{
  p3 = 0;
  p4 = 1;
}

void right_stop() // ?????
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

void pwm_out_left_moto(void)     //?????
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

void pwm_out_right_moto(void)   //?????
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

extern unsigned char Left_Speed_Ratio;  //?????????
extern unsigned char Right_Speed_Ratio; //?????????
unsigned int time = 0;

// ????(??????)
void stop()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_stop();
  right_stop();
}

// ????(??????)
void run()
{
//	OLED_CLS();
//	OLED_P16x16Ch(56, 4, 32);
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_go();
  right_go();
}

// ????(?????,?????)
void left()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_stop();
  right_go();
}

// ????(?????,?????)
void right()
{
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_go();
  right_stop();
}

// ????(??????)
void back()
{
//	OLED_CLS();
//	OLED_P16x16Ch(56, 4, 33);
  push_val_left = Left_Speed_Ratio;
  push_val_right = Right_Speed_Ratio;
  left_back();
  right_back();
}

// ??????????
void Timer0Init()    //????????
{
  TMOD |= 0X01; //??????0??,????1,??TR0?????

  TH0 = 0XFC; //???????,??1ms
  TL0 = 0X18;
  ET0 = 1; //?????0????
  EA = 1; //?????
  TR0 = 1; //?????
}

// ???????
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

void timer0()interrupt 1 using 2  //???????,?????1ms??????,?PWM???????
{
  TH0 = 0XFC; //???????,??1ms
  TL0 = 0X18;
  time++;
  pwm_val_left++;
  pwm_val_right++;
  pwm_out_left_moto();
  pwm_out_right_moto();
}

// ???????(1???)
//void delay1s(void)
//{
//  unsigned char a, b, c;
//  for (c = 167; c > 0; c--)
//    for (b = 171; b > 0; b--)
//      for (a = 16; a > 0; a--);
//  _nop_();
//}

void xunji()  // ????
{
  if (leftled == 0 && rightled == 1) // ????????????
  {
    left();
  }
  else if (leftled == 1 && rightled == 0) // ??????
  {
    right();
  }

  else  //if(leftled ==1 && rightled ==1)
  {
    run();
  }
}

void bizhang()  // ????
{
  if (Left_bizhang == 1 && Right_bizhang == 1)
  {
    run(); 
  }
  else if (Left_bizhang == 1 && Right_bizhang == 0)
  {
    left();  
  }
  else   if (Left_bizhang == 0 && Right_bizhang == 1)
  {
    right();  
  }
  else if (Left_bizhang == 0 && Right_bizhang == 0)
  {
    back();
  }
}

void main(void)
{
  OLED_Init();
  UART_INIT();
  Timer0Init();
  Left_Speed_Ratio = 5;  // ?????????????50%
  Right_Speed_Ratio = 5; // ?????????????50%
  while (1)
  {
		if (r_buf == '5')  // ??
    {
      Left_Speed_Ratio = 5;  // Speed UP?????????????50%
      Right_Speed_Ratio = 5; // ?????????????50%
    }
    if(r_buf == '6')  // ??
    {
      Left_Speed_Ratio = 2;  // Speed Down    ?????????????20%
      Right_Speed_Ratio = 2; // ?????????????20%
    }
    if (r_buf == 'a')
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(48, 4, "A");
			}
      run();
    }
    if (r_buf == '2')
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(56, 4, "B");
			}
      back();
    }
    if (r_buf == '3')
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(56, 4, "C");
			}
      left();
    }
    if (r_buf == '4')
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(55, 4, "D");
			}
      right();
    }
    if (r_buf == '0')
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
			}
      stop();
    }
    if(r_buf == '7')  // ??????
    {
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(55, 4, "E");
			}
			OLED_delay(10);
      Left_Speed_Ratio = 2;			// ?????????????20%
      Right_Speed_Ratio = 2; // ?????????????20%
			xunji();
    }
		if(r_buf == '8') 
		{
			if(r_buf != temp) {
				temp = r_buf;
				OLED_CLS();
				OLED_P8x16Str(32, 4, "F");
			}
			OLED_delay(10);
      Left_Speed_Ratio = 2;			// ?????????????20%
      Right_Speed_Ratio = 2;
			bizhang();
		}
    //    run();
    //    delay1s(); delay1s();  delay1s();  delay1s();  delay1s();
    //    back();
    //    delay1s(); delay1s();  delay1s();  delay1s();  delay1s();
    //    left();
    //    delay1s(); delay1s();  delay1s();  delay1s();  delay1s();
    //    right();
    //    delay1s(); delay1s();  delay1s();  delay1s();  delay1s();
    //    stop();
    //    delay1s(); delay1s();  delay1s();  delay1s();  delay1s();
  }
}

void UARTInterrupt(void) interrupt 4
{
  if (RI)
  {
    RI = 0;
    r_buf = SBUF;
/*    if (r_buf == '5')  // ??
    {
      Left_Speed_Ratio = 5;  // Speed UP?????????????50%
      Right_Speed_Ratio = 5; // ?????????????50%
    }
    if(r_buf == '6')  // ??
    {
      Left_Speed_Ratio = 2;  // Speed Down    ?????????????20%
      Right_Speed_Ratio = 2; // ?????????????20%
    }
    if (r_buf == '1')
    {
//      OLED_P16x16Ch(56, 4, 32);//?°
      run();
    }
    if (r_buf == '2')
    {
      back();
    }
    if (r_buf == '3')
    {
      left();
    }
    if (r_buf == '4')
    {
      right();
    }
    if (r_buf == '0')
    {
      stop();
    }
    if(r_buf == '7')  // ??????
    {
      Left_Speed_Ratio = 2;  // ?????????????20%
      Right_Speed_Ratio = 2; // ?????????????20%
 //     xun_ji = 1;
			xunji();
    }
		/*
    while(xun_ji)
    {
     
			
			
    }
		*//*
    if(r_buf == '8')  // ??????
    {
      Left_Speed_Ratio = 2;  // ?????????????20%
      Right_Speed_Ratio = 2; // ?????????????20%
      bi_zhang = 1;
    }
    while(bi_zhang)
    {
      if(r_buf == 'r')  // ??,?????
      {
        stop();
        bi_zhang = 0;
        break;
      }
      else
      {
        bizhang();
      }
    }  */
  }
}
