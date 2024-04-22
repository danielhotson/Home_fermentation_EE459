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
#include <avr/interrupt.h>
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
#define BUTTON PCINT0	  // Button of Rotary Encoder
#define CHANNEL_B PCINT1  // Channel B of Rotary Encoder
#define CHANNEL_A PCINT2  // Channel A of Rotary Encoder
#define OUTBUFSIZE 20
#define RTC 0xD0

int temp = 0; //in degf times 10
int day;
int hrs;
int min;
int sec;
int lastSec;
int lowerThreshold = 75;
int upperThreshold = 80;
int isHeating = 0;

unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
int count = 0;		// Count to display
unsigned char a, b;
unsigned char pin;

// variables for Rotary Encoder Button
volatile unsigned char buttonPressed = 0;
volatile unsigned char buttonChanged = 0;


/*
	Initialize all of the components (LCD, RTC, Temperature Sensor)
*/
void Init(void){
	rtc_init();  // RTC must be initialized before lcd, otherwise it stalls
	lcd_init();  // Initialize LCD
	ds_init();	 // Initialze Temperature Sensor
	ds_convert();

	
	rtc_load(0x00, 0x00, 0x00, 0x00) // NOTE: Initializing the Registers to 00:00:00:00 for the RTC, can be changed as needed

	DDRD |= (1 << RELAY); //Initialize Relay

	// Initialize Button and Rotary Encoder
	DDRB &= ~(1 << BUTTON);											 // Set BUTTON pin to input
	PORTB |= ((1 << CHANNEL_A) | (1 << CHANNEL_B) | (1 << BUTTON));  // set pull-up resistor for rotary encoder and button
    PCICR |= (1 << BUTTON);
    PCMSK0 |= ((1 << CHANNEL_A) | (1 << CHANNEL_B) | (1 << BUTTON)); // interrupts for rotary encoder and button
	sei();															 // turn global interrupts on

	pin = PINB;

	a = pin & (1 << CHANNEL_A);
	b = pin & (1 << CHANNEL_B);

    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    //new_state = old_state; // I'm 85% sure this is unnecessary 
}


int main(void)
{
	Init();

	// infinite loop
    while (1) {
		CheckInputs();
	}

    return 0;   /* never reached */
}



void CheckInputs(void){
	unsigned char tdata[2];

	// check temperature
	if(ds_temp(tdata)){
		int c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
		temp = (c16 * 9) / 8 + 320;  // F * 10
		ds_convert();
	}

	// get time
	sec = bcd_to_decimal(rtc_read_seconds());

	// Update screen (once a second)
	if(sec != lastSec ){ 
		day = bcd_to_decimal(rtc_read_days());
		hrs = bcd_to_decimal(rtc_read_hours());
		min = bcd_to_decimal(rtc_read_minutes());
		lastSec = sec;
		UpdateScreen();
		UpdateOutputs();
	}
}

void UpdateScreen(void){
	lcd_clear();
	lcd_movetoline(0);

	// Format Temperature
	char outbuf[OUTBUFSIZE];
	int fint = temp / 10;
	int ffrac = temp % 10;
	if (ffrac < 0)
		ffrac = -ffrac;
	snprintf(outbuf, OUTBUFSIZE, "Temp:%3d.%1d", fint, ffrac);
	lcd_stringout(outbuf);

	// Warn User if heating element is on
	if(isHeating){
		lcd_nextLine();
		lcd_stringout("HEATING");
	}

	// Print Elapsed Time
	char time_conv[10]; 
	sprintf(time_conv, "%02d:%02d:%02d:%02d", day, hrs, min, sec);
	lcd_nextLine();
	lcd_stringout("Time Elapsed:");
	lcd_nextLine();
	lcd_stringout(time_conv);
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

ISR(PCINT0_vect){
	pin = PINB;
	a = pin & (1<<CHANNEL_A);
	b = pin & (1<<CHANNEL_B);

	/*
		Button press of Rotary Encoder
	*/
	if (PINB & (1 << BUTTON))
    {
        if (buttonPressed == 0)
        {
            buttonPressed = 1;
        }
        
    }
    else
    {
        if (buttonPressed == 1)
        {
            buttonPressed = 0;
            buttonChanged = 1;
        }
        
    }

	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the count value.
	if (old_state == 0) {

	    // Handle A and B inputs for state 0
		if (a){
			new_state = 1;
		}
		else if (b){
			new_state = 2;
		}

	}
	else if (old_state == 1) {

	    // Handle A and B inputs for state 1
		if (b){
			new_state = 3;
		}
		else if (!a){
			new_state = 0;
		}

	}
	else if (old_state == 2) {

	    // Handle A and B inputs for state 2
		if (!b){
			new_state = 0;
		}
		else if (a){
			new_state = 3;
		}

	}
	else {   // old_state = 3

	    // Handle A and B inputs for state 3
		if (!a){
			new_state = 2;
			count++;
		}
		else if (!b){
			new_state = 1;
			count--;
		}

	}
	// If state changed, update the value of old_state,
	// and set a flag that the state has changed.
	if (new_state != old_state) {
	    changed = 1;
	    old_state = new_state;
	}
}