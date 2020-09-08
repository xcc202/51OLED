#include "intrins.h"
#include "codetab.h"
#include "LQ12864.h"

void main(void)
{
	OLED_Init(); //OLED???
	while(1)
	{
		Draw_BMP(0,0,128,8,CXK);
		OLED_delay(1000);
		OLED_CLS();
		OLED_delay(1000);
	}
}