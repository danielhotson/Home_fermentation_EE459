/*
  rtc.c - Routines for Real Time Clock via i2C
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h> // used to output to lcd for debugging, could be removed
#include "i2c.h"
#include "rtc.h"
#include "lcd.h" // used for debugging, could be removed

#define FOSC 32768
#define BAUD 9600
#define BDIV (FOSC / 100000 - 16) / 2 + 1  // Puts I2C rate below 100kHz
#define RTC_ADDRESS 0xD0 // 0b10100000

unsigned char WDATA[4]; // initialize the write data (no more than size 4)
uint8_t RDATA[2]; // initialize the read data (no more than size 2?)

/*
  rtc_init - Setup the Real Time Clock for receiving and transmitting data
*/
void rtc_init(void)
{
  i2c_init(BDIV);
  _delay_ms(500);
  
}


/*
  rtc_load(seconds, minutes, hours) loads the given values to the RTC
*/
void rtc_load(unsigned char sec, unsigned char min, unsigned char hr)
{

  /*
    WDATA will be an array of the data to be written, with the first element
    indicating where we want to begin writing (should correspond to 'seconds' register)
  */
  WDATA[0] = 3;  // the first address that will be written to
  WDATA[1] = sec;
  WDATA[2] = min;
  WDATA[3] = hr;

  /*
    write 3 elements from WDATA [1-3] to location RTC_ADDRESS,
    starting from element WDATA[0]
  */
  i2c_io(RTC_ADDRESS, WDATA, 4, NULL, 0);

  return;
}

/*
  Returns value in the "seconds" register (register 3)
    Known Issue: skipping at the following values
      9->16, 25->32, 41->48, 57->64, 73->80
    But it does correctly reset after 1 minute
*/
uint8_t rtc_read_seconds(void)
{

  WDATA[0] = 3;
  
  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);

  return RDATA[0]; // return the seconds value (not sure what index it should be)
}

/*
  Returns value in the "minutes" register (register 4)
*/
uint8_t rtc_read_minutes(void)
{
  WDATA[0] = 4;

  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);
  
  return RDATA[0];
}

/*
  Returns value in the "hours" register (register 5)
*/
uint8_t rtc_read_hours(void)
{
  WDATA[0] = 5;

  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);

  return RDATA[0];
}

/*
  Returns value in the "days" register (register 6)
*/
uint8_t rtc_read_days(void)
{
  WDATA[0] = 6;

  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);

  return RDATA[0];
}

/*
  Because the RTC sends register values coded in BCD (Binary Coded Decimal) we need to convert it to decimal for our purposes
*/
int bcd_to_decimal(unsigned char x)
{
  return x - 6 * (x >> 4);
}