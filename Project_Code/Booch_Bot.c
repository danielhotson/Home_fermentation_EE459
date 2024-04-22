/*************************************************************
*       Booch_Bot.c main project code for EE 459
*
*       Program loops through a state machine to operate booch_bot system
*
* Revision History
* Date     Author      Description
* 03/28/24 D.Hotson    Initial structure
* 
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h"
#include "i2c.h"
#include "ds18b20.h"
#include "relay.h"
#include "rtc.h"
#include "Booch_Bot.h"

#define RELAY PD2
#define LED PC0
#define ENCODER_SWITCH PB0
#define ENCODER_1 PB1
#define ENCODER_2 PB2
#define OUTBUFSIZE 20
#define RTC 0xD0

char lastScreen[4][20];
char nextScreen[4][20];
int temp = 0; //in degf times 10
int lastTemp = 0;
int lowerThreshold = 75;
int upperThreshold = 80;
int isHeating = 0;



int main(void)
{
	Init();

	//infinite loop
    while (1) {
		CheckInputs();
	}

    return 0;   /* never reached */
}

/*
	Initialize all of the components (LCD, RTC, Temperature Sensor)
*/
void Init(void){
	lcd_init();
	rtc_init();
	ds_init();
	ds_convert();
	DDRD |= (1 << RELAY);
}

void CheckInputs(void){
	unsigned char tdata[2];

	/*

	*/
	if(ds_temp(tdata)){
		int c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
		temp = (c16 * 9) / 8 + 320;  // F * 10
		ds_convert();
	}

	/*
		add time polling
	*/

	/*
		update the screen and outputs if time or temperature changes (every minute)
	*/
	if(temp != lastTemp){
		UpdateScreen();
		UpdateOutputs();
	}
	lastTemp = temp;
}
void UpdateScreen(void){
	lcd_clear();
	lcd_movetoline(1);
	char outbuf[OUTBUFSIZE];
	int fint = temp / 10;
	int ffrac = temp % 10;
	if (ffrac < 0)
	ffrac = -ffrac;
	snprintf(outbuf, OUTBUFSIZE, "    Temp:%3d.%1d", fint, ffrac);
	lcd_stringout(outbuf);
	if(isHeating){
		lcd_nextLine();
		lcd_stringout("HEATING");
	}
}

void UpdateOutputs(void){
	if (isHeating == 1){
		if(temp > 790){
			relay_off(RELAY);
			isHeating = 0;
		}
	}
	else{
		if(temp < 750){
			isHeating = 1;
			relay_on(RELAY);
		}
	}
}

void ScreenDisplay(void){
	char time_conv[10]; // this will be the converted int -> string for printing to lcd

	sprintf(time_conv, "%02d:%02d:%02d:%02d",
	 	bcd_to_decimal(rtc_read_days()),
		bcd_to_decimal(rtc_read_hours()),
		bcd_to_decimal(rtc_read_minutes()),
		bcd_to_decimal(rtc_read_seconds()));
	lcd_movetoline(0);
	lcd_stringout("Time Elapsed:");
	lcd_movetoline(1);
	lcd_stringout(time_conv);

	char temp_str[10];
	sprintf(temp_str, "%03d", temp);
	lcd_movetoline(2);
	lcd_stringout("Temperature");
	lcd_movetoline(3);
	lcd_stringout(temp_str);

}