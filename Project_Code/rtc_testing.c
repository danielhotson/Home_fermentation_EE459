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

  //lcd_stringout("RTC initialized...");

  // may want to consider changing the values here to 0xXX format (unsigned chars)
  rtc_load(0x30,0x59,0x00); // set the seconds to 0, minutes to 0, and hours to 0

  char time_conv[10]; // this will be the converted int -> string for printing to lcd


  /*
    Continuously prints out the elapsed time
  */
  while(1)
  {

    
    //  This successfully prints stuff out
    /*
    sprintf(time_conv, "%02d", bcd_to_decimal(rtc_read_seconds())); // this should read the BCD value from RTC, convert to Decimal, then to string
    lcd_movetoline(1);
    lcd_stringout(time_conv); // this should print that string out to the LCD
    _delay_ms(1000);
    */

   /*
    Let's see if we can format it as
    Time Elapsed:
    HH:MM:SS
   */

    sprintf(time_conv, "%02d:%02d:%02d", bcd_to_decimal(rtc_read_hours()), bcd_to_decimal(rtc_read_minutes()), bcd_to_decimal(rtc_read_seconds()));
    lcd_movetoline(0);
    lcd_stringout("Time Elapsed:");
    lcd_movetoline(1);
    lcd_stringout(time_conv);
    _delay_ms(1000);
  }

  return 0; // Never Reached
}