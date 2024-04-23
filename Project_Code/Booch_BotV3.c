/*
    Main Program combining all the components
    Uses EEPROM
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "i2c.h"
#include "ds18b20.h"
#include "relay.h"
#include "rtc.h"
#include "Booch_BotV3.h"

#define GREEN_LED PC0
#define RELAY PD2
#define LED PC0
#define BUTTON PCINT0	  // Button of Rotary Encoder
#define CHANNEL_B PCINT1  // Channel B of Rotary Encoder
#define CHANNEL_A PCINT2  // Channel A of Rotary Encoder
#define OUTBUFSIZE 20
#define RTC 0xD0

// addresses in EEPROM for following values
#define EEPROM_STATE_ADDR 200
#define EEPROM_DAYS_ADDR 250
#define EEPROM_HOURS_ADDR 300
#define EEPROM_MINS_ADDR 350
#define EEPROM_SECONDS_ADDR 400
#define EEPROM_UPBOUND_ADDR 450
#define EEPROM_LOBOUND_ADDR 500
#define EEPROM_TIMEBOUND_ADDR 550

#define INITIALIZING 0
#define START_MENU 1
#define BREWING 2
#define FINISHED_BREWING 3

unsigned char new_state, old_state, pin, a, b; // variables for Rotary Encoder

volatile unsigned char changed = 0, buttonPressed = 0, buttonChanged = 0; // variables for Rotary Encoder Button
volatile int count = 0; // variable for rotary encoder value

volatile short state = START_MENU; // state of the booch bot
int isHeating = 0; //is relay on

int last_seconds = 0; 

int temperature;
volatile short upperTempBound = 0;
volatile short lowerTempBound = 0;
volatile short timeBound = 0;

volatile short days;
volatile short hrs;
volatile short min;
volatile short sec;


int main(void)
{
    // Initialize everything
    Initialize();

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
            PORTC |= 1 << GREEN_LED;
            _delay_ms(500);
            PORTC &= ~(1 <<GREEN_LED);
            _delay_ms(500);
            if (buttonChanged){
                Initialize();
                
            }
        }
    }

    return 0; // never reached
}

void StartMenu(void)
{
    char temp[5]; // temporary value DEBUGGING

    lcd_movetoline(0);
    lcd_stringout("Ferment Time min: ");

    while (state == START_MENU)
    {
        if (buttonChanged) // checks for button press
        {   
            if(timeBound == 0 && count != 0){
                timeBound = count;
                count = 0;
                eeprom_update_word((void *) 550, timeBound);
                lcd_clear();
                lcd_stringout("Lower Temp f:");
                _delay_ms(100);
            }
            else if(lowerTempBound == 0 && count != 0){
                lowerTempBound = count*10;
                count = 0;
                eeprom_update_word((void *) 500, lowerTempBound); 
                lcd_clear();
                lcd_stringout("Upper Temp f:");
                _delay_ms(100);
            }
            else if(upperTempBound == 0 && count != 0){
                upperTempBound = count*10;
                if(upperTempBound < lowerTempBound){
                    upperTempBound = lowerTempBound + 1;
                }
                eeprom_update_word((void *) 450, upperTempBound); 
                lcd_clear();
                count = 0;
                lcd_stringout("Brewing state");
                _delay_ms(1000);
                state = BREWING;
                rtc_load(0x00, 0x00, 0x00, 0x00); // Reset the RTC
            }
            eeprom_update_word((void *) 200, state);
            buttonChanged = 0;
        }
        else if (changed) // checks for rotation
        {
            lcd_movetoline(1);
            if(count < 0){
                count = 0;
            }
            sprintf(temp, "%03d", count);
            lcd_stringout(temp);
            changed = 0;
        }
        
    }
    return;
}

void Brewing(void)
{
    unsigned char tdata[2];
    char converted_time[20]; // the array that the time will be converted into
    lcd_clear();

    while (state == BREWING)
    {   
        if(ds_temp(tdata)){
		    int c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
		    temperature = (c16 * 9) / 8 + 320;  // F * 10
		    ds_convert();
	    }

        // this should probably all done simultaneously for consiseness + synchronization
        sec = bcd_to_decimal(rtc_read_seconds()); // sample the time so it only updates once a second
        min = bcd_to_decimal(rtc_read_minutes());
        hrs = bcd_to_decimal(rtc_read_hours());
        days = bcd_to_decimal(rtc_read_days());

        // Format Temperature
        char outbuf[OUTBUFSIZE];
	    int fint = temperature / 10;
	    int ffrac = temperature % 10;
	    if (ffrac < 0)
	    	ffrac = -ffrac;
	    snprintf(outbuf, OUTBUFSIZE, "Temperature:%3d.%1d", fint, ffrac);

        if(sec != last_seconds) // update screen at most once a second
        {
            lcd_clear();
            lcd_movetoline(0);
            lcd_stringout("Elapsed Time:");
            sprintf(converted_time, "%02d:%02d:%02d:%02d", days, hrs, min, sec);
            lcd_movetoline(1);
            lcd_stringout(converted_time);

            last_seconds = sec;

            //print temperature
            lcd_movetoline(2);
            lcd_stringout(outbuf);

            //check relay
            if (isHeating == 1){
                if(temperature > upperTempBound){
                    relay_off(RELAY);
                    isHeating = 0;
		    }
            }
            else{
                if(temperature < lowerTempBound){
                    isHeating = 1;
                    relay_on(RELAY);
                }
            }

            // Warn User if heating element is on
            if(isHeating){
                lcd_nextLine();
                lcd_stringout("HEATING");
            }
            eeprom_update_word((void *) 200, state);
            eeprom_update_word((void *) 250, days); 
            eeprom_update_word((void *) 300, hrs); 
            eeprom_update_word((void *) 350, min); 
            eeprom_update_word((void *) 400, sec); 
        }
        // display time/temperature updates
        // regulate temperature
        
        // add a break function based on time (timer)
        if (min == timeBound){
            lcd_clear();
            lcd_movetoline(0);
            lcd_stringout("DONE");
            lcd_nextLine();
            if(!(PINC & (1 << PC1))){
                lcd_stringout("Alc may be present");
                lcd_nextLine();
                lcd_stringout("Drink with Caution");
            }
            lcd_nextLine();
            lcd_stringout("Press to reset");
            state = FINISHED_BREWING;
            eeprom_update_word((void *) 200, state);
            eeprom_update_word((void *) 250, 0); 
            eeprom_update_word((void *) 300, 0); 
            eeprom_update_word((void *) 350, 0); 
            eeprom_update_word((void *) 400, 0);
            eeprom_update_word((void *) 450, 0);
            eeprom_update_word((void *) 500, 0);
            eeprom_update_word((void *) 550, 0);
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
    
    //Initialize Green LED output
    DDRC |= (1 << DDC0);

 // init as input
     DDRC &= ~(1 << DDC1);
     DDRC &= ~(1 << DDC2 );

    //get old data from eeprom
    state = eeprom_read_word((void *) 200);

    if(state == BREWING){
        days = eeprom_read_word((void *) 250); 
        hrs = eeprom_read_word((void *) 300); 
        min = eeprom_read_word((void *) 350); 
        sec = eeprom_read_word((void *) 400); 
        upperTempBound = eeprom_read_word((void *) 450); 
        lowerTempBound = eeprom_read_word((void *) 500); 
        timeBound = eeprom_read_word((void *) 550);

        // these new values need to be converted into BCD format, and the rtc_load expects an unsigned char
        rtc_load(decimal_to_bcd(sec), decimal_to_bcd(min), decimal_to_bcd(hrs), decimal_to_bcd(days));
    }
    else {
        eeprom_update_word((void *) 200, START_MENU);
        eeprom_update_word((void *) 250, 0); 
        eeprom_update_word((void *) 300, 0); 
        eeprom_update_word((void *) 350, 0); 
        eeprom_update_word((void *) 400, 0);
        eeprom_update_word((void *) 450, 0);
        eeprom_update_word((void *) 500, 0);
        eeprom_update_word((void *) 550, 0);

        state = eeprom_read_word((void *) 200);
        days = eeprom_read_word((void *) 250); 
        hrs = eeprom_read_word((void *) 300); 
        min = eeprom_read_word((void *) 350); 
        sec = eeprom_read_word((void *) 400); 
        upperTempBound = eeprom_read_word((void *) 450); 
        lowerTempBound = eeprom_read_word((void *) 500); 
        timeBound = eeprom_read_word((void *) 550);

        rtc_load(0x00,0x00,0x00,0x00); // reset the RTC's internal registers
    }
    



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

unsigned char decimal_to_bcd(short decimal)
{
    short converted = (decimal/10*16) + (dec%10);
    unsigned char temp[10];
    sprintf(temp, "%d", converted);

    return temp;
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