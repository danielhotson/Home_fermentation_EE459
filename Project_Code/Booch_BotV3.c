/*
    Main Program combining all the components
    Uses EEPROM
*/

#include "Booch_BotV3.h"
#include "ds18b20.h"
#include "i2c.h"
#include "lcd.h"
#include "relay.h"
#include "rtc.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#define GREEN_LED PC0
#define RELAY PD2
#define BUTTON PCINT0    // Button of Rotary Encoder
#define CHANNEL_B PCINT1 // Channel B of Rotary Encoder
#define CHANNEL_A PCINT2 // Channel A of Rotary Encoder
#define BUFSIZE 20
#define RTC 0xD0

// addresses in EEPROM for following values
#define EEPROM_STATE_ADDR 200
#define EEPROM_UPBOUND_ADDR 300
#define EEPROM_LOBOUND_ADDR 400
#define EEPROM_TIMEBOUND_ADDR 500
#define INITIALIZING 0
#define START_MENU 1
#define BREWING 2
#define FINISHED_BREWING 3

int isHeating = 0; // is relay on
int last_seconds = 0;
int temperature;
short days;
short hrs;
short min;
short sec;

// Variables accessed in ISR
volatile unsigned char new_state, old_state, pin, a,
    b; // variables for Rotary Encoder
volatile unsigned char changed = 0, buttonPressed = 0,
                       buttonChanged = 0; // variables for Button
volatile int count = 0;                   // Rotary encoder value
// EEPROM variables
volatile short state = START_MENU; // state of the booch bot
volatile short upperTempBound = 0;
volatile short lowerTempBound = 0;
volatile short timeBound = 0;

int main(void) {
  // Initialize everything
  Initialize();

  while (1) {
    // Check what the state is
    if (state == START_MENU) {
      StartMenu();
    }

    else if (state == BREWING) {
      Brewing();
    }

    else { // state == FINISHED_BREWING
      PORTC |= 1 << GREEN_LED;
      _delay_ms(500);
      PORTC &= ~(1 << GREEN_LED);
      _delay_ms(500);

      if (buttonChanged) {
        Initialize();
      }
    }
  }

  return 0; // never reached
}

void Initialize(void) { // Initialize all of the modules
  state = INITIALIZING;

  rtc_init();   // Initialize the PCF8523 RTC (Must be done first)
  lcd_init();   // Initialize the LCD
  ds_init();    // Initialize the DS temperature Sensor
  ds_convert(); // Start a Temperature reading

  rotary_encoder_init(); // Initialize Rotary Encoder

  DDRC |= (1 << DDC0);  // Green LED output
  DDRD |= (1 << RELAY); // Initialize the Relay
  DDRC &= ~(1 << DDC1); // Ethanol sensor input

  state = eeprom_read_word((void *)EEPROM_STATE_ADDR); // get state from eeprom

  if (state == BREWING) {
    upperTempBound = eeprom_read_word((void *)EEPROM_UPBOUND_ADDR);
    lowerTempBound = eeprom_read_word((void *)EEPROM_LOBOUND_ADDR);
    timeBound = eeprom_read_word((void *)EEPROM_TIMEBOUND_ADDR);
  }

  else {
    eeprom_update_word((void *)EEPROM_STATE_ADDR, START_MENU);
    eeprom_update_word((void *)EEPROM_UPBOUND_ADDR, 0);
    eeprom_update_word((void *)EEPROM_LOBOUND_ADDR, 0);
    eeprom_update_word((void *)EEPROM_TIMEBOUND_ADDR, 0);

    state = eeprom_read_word((void *)EEPROM_STATE_ADDR);
    upperTempBound = eeprom_read_word((void *)EEPROM_UPBOUND_ADDR);
    lowerTempBound = eeprom_read_word((void *)EEPROM_LOBOUND_ADDR);
    timeBound = eeprom_read_word((void *)EEPROM_TIMEBOUND_ADDR);

    rtc_load(0x00, 0x00, 0x00, 0x00); // reset the RTC's internal registers
  }

  // End Initialization
  _delay_ms(500);

  return;
}

void StartMenu(void) {
  char temp[5]; // temporary value
  lcd_movetoline(0);
  lcd_stringout("Ferment Time min: ");

  while (state == START_MENU) {
    if (buttonChanged) {                  // Checks for button press
      if (timeBound == 0 && count != 0) { // Record Ferment Time
        timeBound = count;
        count = 0;
        eeprom_update_word((void *)EEPROM_TIMEBOUND_ADDR, timeBound);
        lcd_clear();
        lcd_stringout("Lower Temp f:"); // Display next prompt
        _delay_ms(100);
      }

      else if (lowerTempBound == 0 && count != 0) { // Record Lower Temp
        lowerTempBound = count * 10;
        count = 0;
        eeprom_update_word((void *)EEPROM_LOBOUND_ADDR, lowerTempBound);
        lcd_clear();
        lcd_stringout("Upper Temp f:"); // Display next prompt
        _delay_ms(100);
      }

      else if (upperTempBound == 0 && count != 0) { // Record Upper Temp
        upperTempBound = count * 10;
        if (upperTempBound < lowerTempBound) {
          upperTempBound = lowerTempBound + 1;
        }
        eeprom_update_word((void *)EEPROM_UPBOUND_ADDR, upperTempBound);
        lcd_clear();
        lcd_stringout("Brewing state");
        _delay_ms(1000);
        state = BREWING;
        eeprom_update_word((void *)EEPROM_STATE_ADDR, state);
        rtc_load(0x00, 0x00, 0x00, 0x00); // Reset the RTC
      }

      buttonChanged = 0;
    }

    else if (changed) { // Update count based on rotary encodeer
      lcd_movetoline(1);
      if (count < 0) {
        count = 0;
      }
      sprintf(temp, "%03d", count);
      lcd_stringout(temp);
      changed = 0;
    }
  }
  return;
}

void Brewing(void) {
  unsigned char tdata[2];
  char converted_time[BUFSIZE]; // array for time
  char converted_temp[BUFSIZE]; // array for temp
  lcd_clear();
  while (state == BREWING) {
    // Get Time
    sec = bcd_to_decimal(rtc_read_seconds());
    min = bcd_to_decimal(rtc_read_minutes());
    hrs = bcd_to_decimal(rtc_read_hours());
    days = bcd_to_decimal(rtc_read_days());

    if (ds_temp(tdata)) {                     // Get temperature if it is ready
      int c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
      temperature = (c16 * 9) / 8 + 320;      // F * 10
      ds_convert();
    }

    if (sec != last_seconds) { // update screen at most once a second
      lcd_clear();
      lcd_movetoline(0);
      lcd_stringout("Elapsed Time:");
      sprintf(converted_time, "%02d:%02d:%02d:%02d", days, hrs, min, sec);
      lcd_movetoline(1);
      lcd_stringout(converted_time);

      last_seconds = sec;

      // Format Temperature
      int fint = temperature / 10;
      int ffrac = temperature % 10;
      if (ffrac < 0)
        ffrac = -ffrac;
      snprintf(converted_temp, BUFSIZE, "Temperature:%3d.%1d", fint, ffrac);

      // print temperature
      lcd_movetoline(2);
      lcd_stringout(converted_temp);

      // check relay
      if (isHeating == 1) {
        if (temperature > upperTempBound) {
          relay_off(RELAY);
          isHeating = 0;
        }
      } else {
        if (temperature < lowerTempBound) {
          isHeating = 1;
          relay_on(RELAY);
        }
      }

      // Warn User if heating element is on
      if (isHeating) {
        lcd_nextLine();
        lcd_stringout("HEATING");
      }
    }

    // add a break function based on time (timer)
    if (min == timeBound) {
      state = FINISHED_BREWING;
      eeprom_update_word((void *)EEPROM_STATE_ADDR, state);
      eeprom_update_word((void *)EEPROM_UPBOUND_ADDR, 0); // Reset User Inputs
      eeprom_update_word((void *)EEPROM_LOBOUND_ADDR, 0);
      eeprom_update_word((void *)EEPROM_TIMEBOUND_ADDR, 0);

      lcd_clear();
      lcd_movetoline(0);
      lcd_stringout("DONE");
      lcd_nextLine();

      if (!(PINC & (1 << PC1))) {
        lcd_stringout("Alc may be present");
        lcd_nextLine();
        lcd_stringout("Drink with Caution");
      }

      lcd_nextLine();
      lcd_stringout("Press to reset");
    }
  }
  return;
}

void rotary_encoder_init(void) {
  DDRB &= ~(1 << BUTTON); // Set BUTTON pin to input
  PORTB |=
      ((1 << CHANNEL_A) | (1 << CHANNEL_B) |
       (1 << BUTTON)); // set pull-up resistor for rotary encoder and button
  PCICR |= (1 << BUTTON);
  PCMSK0 |= ((1 << CHANNEL_A) | (1 << CHANNEL_B) |
             (1 << BUTTON)); // interrupts for rotary encoder and button
  sei();                     // turn global interrupts on

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

// Rotery encoder / button interrupts
ISR(PCINT0_vect) {
  pin = PINB;
  a = pin & (1 << CHANNEL_A);
  b = pin & (1 << CHANNEL_B);

  if (PINB & (1 << BUTTON)) { //    Button press of Rotary Encoder
    if (buttonPressed == 0) {
      buttonPressed = 1;
    }
  } else {
    if (buttonPressed == 1) {
      buttonPressed = 0;
      buttonChanged = 1;
    }
  }

  // For each state, examine the two input bits to see if state
  // has changed, and if so set "new_state" to the new state,
  // and adjust the count value.
  if (old_state == 0) {
    if (a)
      new_state = 1;
    else if (b)
      new_state = 2;
  } else if (old_state == 1) {
    // Handle A and B inputs for state 1
    if (b)
      new_state = 3;
    else if (!a)
      new_state = 0;
  } else if (old_state == 2) {
    // Handle A and B inputs for state 2
    if (!b)
      new_state = 0;
    else if (a)
      new_state = 3;
  } else { // old_state = 3
    // Handle A and B inputs for state 3
    if (!a) {
      new_state = 2;
      count++;
    } else if (!b) {
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

unsigned char decimal_to_bcd(short decimal) {
  short converted = (decimal / 10 * 16) + (decimal % 10);
  unsigned char temp[10];
  sprintf(temp, "%d", converted);
  return temp;
}