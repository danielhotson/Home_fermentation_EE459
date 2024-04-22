/*
    Main Program combining all the components
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "i2c.h"
#include "ds18b20.h"
#include "relay.h"
#include "rtc.h"
#include "Booch_BotV2.h"

#define RELAY PD2
#define LED PC0
#define BUTTON PCINT0	  // Button of Rotary Encoder
#define CHANNEL_B PCINT1  // Channel B of Rotary Encoder
#define CHANNEL_A PCINT2  // Channel A of Rotary Encoder
#define OUTBUFSIZE 20
#define RTC 0xD0

#define INITIALIZING 0
#define START_MENU 1
#define BREWING 2
#define FINISHED_BREWING 3

unsigned char new_state, old_state, pin, a, b; // variables for Rotary Encoder

volatile unsigned char changed = 0, buttonPressed = 0, buttonChanged = 0; // variables for Rotary Encoder Button
volatile int count = 0; // variable for rotary encoder value

int state; // state of the booch bot

int current_seconds = 0, last_seconds = 0; 

int temperature;


int main(void)
{
    // Initialize everything
    Initialize();
    state = START_MENU;

    while(1)
    {
        // Check what the state is
        if (state == START_MENU)
        {
            StartMenu();
        }
        else if (state == BREWING)
        {
            Brewing();
        }
        else // state == FINISHED_BREWING
        {
            _delay_ms(100);
        }
    }

    return 0; // never reached
}

void StartMenu(void)
{
    char temp[5]; // temporary value DEBUGGING


    lcd_movetoline(0);
    lcd_stringout("Start Menu...");
    while (state == START_MENU)
    {
        if (buttonChanged)
        {
            lcd_movetoline(0);
            lcd_stringout("-");
            state = BREWING;
        }
        else if (changed)
        {
            lcd_movetoline(1);
            sprintf(temp, "%03d", count);
            lcd_stringout(temp);

            /*
                Add Code Here
            */

            changed = 0;
        }
        
        //else
        //{
            //lcd_stringout("-");
        //}
    }
    return;
}

void Brewing(void)
{
    unsigned char tdata[2];

    int days, hours, minutes;
    char converted_time[4]; // the array that the time will be converted into
    lcd_clear();
    rtc_load(0x00, 0x00, 0x00, 0x00); // Reset the RTC

    // This doesn't need to be repeatedly printed
    lcd_movetoline(0);
    lcd_stringout("Elapsed Time:");
    //lcd_movetoline(2);
    //lcd_stringout("Temperature: ");

    while (state == BREWING)
    {
        if(ds_temp(tdata)){
		    int c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
		    temperature = (c16 * 9) / 8 + 320;  // F * 10
		    ds_convert();
	    }

        // this should probably all done simultaneously for consiseness + synchronization
        current_seconds = bcd_to_decimal(rtc_read_seconds()); // sample the time so it only updates once a second
        minutes = bcd_to_decimal(rtc_read_minutes());
        hours = bcd_to_decimal(rtc_read_hours());
        days = bcd_to_decimal(rtc_read_days());

        // Format Temperature
        char outbuf[OUTBUFSIZE];
	    int fint = temperature / 10;
	    int ffrac = temperature % 10;
	    if (ffrac < 0)
	    	ffrac = -ffrac;
	    snprintf(outbuf, OUTBUFSIZE, "Temperature:%3d.%1d", fint, ffrac);

        if(current_seconds != last_seconds) // update screen at most once a second
        {
            sprintf(converted_time, "%02d:%02d:%02d:%02d", days, hours, minutes, current_seconds);
            lcd_movetoline(1);
            lcd_stringout(converted_time);

            //print temperature
            lcd_movetoline(3);
            lcd_stringout(outbuf);
            
        }
        // display time/temperature updates
        // regulate temperature
        
        // add a break function based on time (timer)
        if (current_seconds == 30){
            lcd_clear();
            lcd_movetoline(0);
            lcd_stringout("DONE");
            state = FINISHED_BREWING;
        }
    }
    return;
}

/*
    Initialize all of the modules
*/
void Initialize(void)
{
    state = INITIALIZING;
    /*
        RTC MUST BE INITIALIZED FIRST, OTHERWISE THE PROGRAM STALLS
            - this is probably from conflicts of declarations or something
    */
    rtc_init();   // Initialize the PCF8523 RTC
    lcd_init();   // Initialize the LCD
    ds_init();    // Initialize the DS temperature Sensor
    ds_convert();
    relay_init(); // Initialize the Relay
    rotary_encoder_init(); // Initialize Rotary Encoder

    //End Initialization
    _delay_ms(500);
    
    return;
}

void rotary_encoder_init(void)
{
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
    
    return;
}

/*
    Initialize the Relay Pin
*/
void relay_init(void)
{
    DDRD |= (1 << RELAY);
}

ISR(PCINT0_vect){
	pin = PINB;
	a = pin & (1 << CHANNEL_A);
	b = pin & (1 << CHANNEL_B);

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