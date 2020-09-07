
//#include "reg52.h"
#include "intrins.h"
#include "codetab.h"
#include "LQ12864.h"

void main(void)
{
	OLED_Init(); //OLED???
	while(1)
	{
//		OLED_Fill(0xff);
//		delay(2000);
//		OLED_Fill(0x00); //???
//		delay(200);
    /*
		OLED_P8x16Str(0,2,"I");
		delay(2000);
		OLED_CLS();
		OLED_P8x16Str(0, 2, "LOVE");
		delay(2000);
		OLED_CLS();
		OLED_P8x16Str(0, 2, "YOU");
		delay(2000);
		OLED_CLS();
		delay(2000);
		OLED_P8x16Str(0, 2, "I LOVE YOU !");
		delay(1000);
		OLED_CLS();
		delay(1000);
		//2
		OLED_P8x16Str(0, 2, "I LOVE YOU !");
		delay(1000);
		OLED_CLS();
		delay(1000);
		//3
		OLED_P8x16Str(0, 2, "I LOVE YOU !");
		delay(1000);
		OLED_CLS();
		delay(3000);
		*/
		
		OLED_P8x16Str(0, 2, "I LOVE YOU !");
		delay(400);
		OLED_CLS();
		delay(400);
		
	}
}