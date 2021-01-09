#include "ssd1306/ssd1306.h"
#include "ssd1306/fonts.h"
#include "main.h"

// show message on OLED display
static void display(char *str)
{
	char delim[] = "\r\n";
	char *ptr = strtok(str, delim);
	SSD1306_Clear();
	u_int16_t y = 0;
	while (ptr!=NULL)
	{
		SSD1306_GotoXY(0, y);
		SSD1306_Puts(ptr, &Font_7x10, 1);
		ptr = strtok(NULL, delim);
		y += 12;
	}
	SSD1306_UpdateScreen();
}

static void displaySim800error(char* cmd, char* result)
{
  char msg[80];
  sprintf(msg, "%s\r\nERROR\r\n%s", cmd, result);
  display(msg);
  HAL_Delay(5000);
}

static void displayBatteryVoltage(float vbat) {
	uint8_t text[20];
	sprintf((char *)&text, "%.2fV", vbat);
	SSD1306_Clear();
	SSD1306_GotoXY(22, 12);
	SSD1306_Puts((char *)&text, &Font_16x26, 1);
	SSD1306_UpdateScreen();
	if (vbat < 3.5)
	{
		sprintf((char *)&text, "LOW BATTERY");
		SSD1306_GotoXY(0, 40);
		SSD1306_Puts((char *)&text, &Font_11x18, 1);
		SSD1306_UpdateScreen();
	}
}
