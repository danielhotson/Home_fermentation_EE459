/*
  rtc.c - Routines for Real Time Clock via i2C
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h> // used to output to lcd for debugging, could be removed
#include "i2c.h"
#include "rtc.h"

#define FOSC 32768
#define BAUD 9600
#define BDIV (FOSC / 100000 - 16) / 2 + 1  // Puts I2C rate below 100kHz
#define RTC_ADDRESS 0xD0 // 0b10100000

unsigned char WDATA[5]; // initialize the write data (no more than size 4)
uint8_t RDATA[2]; // initialize the read data (no more than size 2?)

/*
  rtc_init - Setup the Real Time Clock for receiving and transmitting data
*/
void rtc_init(void)
{
  i2c_init(BDIV);
  _delay_ms(100);
  return;
}

/*
  Resets the time of the RTC to 00:00:00:00 (DD:HH:MM:SS)
*/
void start_timer(void)
{
  rtc_load(0x00, 0x00, 0x00, 0x00);

  return;
}


/*
  rtc_load(seconds, minutes, hours) loads the given values to the RTC
*/
void rtc_load(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char days)
{

  /*
    WDATA will be an array of the data to be written, with the first element
    indicating where we want to begin writing (should correspond to 'seconds' register)
  */
  WDATA[0] = 3;  // the first address that will be written to
  WDATA[1] = seconds;
  WDATA[2] = minutes;
  WDATA[3] = hours;
  WDATA[4] = days;

  /*
    write 4 elements from WDATA [1-4] to location RTC_ADDRESS,
    starting from element WDATA[0]
  */
  i2c_io(RTC_ADDRESS, WDATA, 5, NULL, 0);

  return;
}

/*
  Returns value in the "seconds" register (register 3) in BCD format
*/
uint8_t rtc_read_seconds(void)
{

  WDATA[0] = 3;
  
  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);

  return RDATA[0];
}

/*
  Returns value in the "minutes" register (register 4) in BCD format
*/
uint8_t rtc_read_minutes(void)
{
  WDATA[0] = 4;

  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);
  
  return RDATA[0];
}

/*
  Returns value in the "hours" register (register 5) in BCD format
*/
uint8_t rtc_read_hours(void)
{
  WDATA[0] = 5;

  i2c_io(RTC_ADDRESS, WDATA, 1, RDATA, 1);

  return RDATA[0];
}

/*
  Returns value in the "days" register (register 6) in BCD format
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