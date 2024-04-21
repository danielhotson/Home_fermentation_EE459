/*
    This will be the initial tests for the real itme clock
*/
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "i2c.h"
#include "rtc.h"

#define RTC_ADDRESS 0xD0 // 0x68 normally, but we need the 8-bit address

int main(void)
{
  rtc_init();
  _delay_ms(100);
  lcd_init();
  _delay_ms(100);

  lcd_stringout("RTC initialized...");


  rtc_load(0,89,9); // set the seconds to 0, minutes to 0, and hours to 0

  char time_conv[10]; // this will be the converted int -> string for printing to lcd


  /*
    This loop should continuously print out the seconds elapsed
  */
  while(1)
  {
    /*
      With the following block of code, the results skip at the following points (consistently)
        9  -> 16
        25 -> 32
        41 -> 48
        57 -> 64
        73 -> 80
        reset after 89
      But it does accurately restart after 1 minute  
    */

    /*
    sprintf(time_conv, "%d",rtc_read_seconds()); // rtc_read_seconds should return a 0, and sprintf should convert it to a string
    lcd_movetoline(1);
    lcd_stringout(time_conv);
    _delay_ms(1000);
    */

    sprintf(time_conv, "%d", bcd_to_decimal(rtc_read_hours())); // this should read the BCD value from RTC, convert to Decimal, then to string
    lcd_movetoline(1);
    lcd_stringout(time_conv); // this should print that string out to the LCD
    _delay_ms(15000);

  }

  return 0; // Never Reached
}