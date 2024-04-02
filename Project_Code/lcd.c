/*
  lcd.c - Routines for sending data and commands to the LCD shield
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"                // Declarations of the LCD functions

#define FOSC 7372890            // Clock frequency = Oscillator freq.
#define BAUD 9600               // UART0 baud rate
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define BDIV (FOSC / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define I2C_ADDRESS 0x7E //0b01111110

/*
  lcd_init - Do various things to initialize the LCD display
*/
void lcd_init(void)
{
  i2c_init(BDIV);

    lcd_writecommand(0x38);  //Function Set: 2 lines
	  _delay_ms(120);

    lcd_writecommand(0x0f); //Display on, cursor on, cursor blinks
	  _delay_ms(120);

    lcd_writecommand(0x01); //Clear display
	  _delay_ms(120);

    lcd_writecommand(0x06); //Entry mode: cursor shifts right
	  _delay_ms(120);

}


/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    int i = 0;
    while (str[i] != '\0') {    // Loop until next charater is NULL byte
        lcd_writedata(str[i]);  // Send the character
        i++;
    }
}

/*
  lcd_writecommand - Output a byte to the LCD command register.
*/
void lcd_writecommand(unsigned char cmd)
{
  unsigned char wbuf[2];
  wbuf[0] = 0x80; //
  wbuf[1] = cmd;

  i2c_io(I2C_ADDRESS, wbuf, 2, NULL , 0); 
}

/*
  lcd_writedata - Output a byte to the LCD data register
*/
void lcd_writedata(unsigned char dat)
{
  unsigned char wbuf[] = dat;

  i2c_io(I2C_ADDRESS, wbuf, 1, NULL , 0); 

}


/*
  lcd_moveto - Move the cursor to the row and column given by the arguments.
  Row is 0 or 1, column is 0 - 15.
*/
void lcd_moveto(unsigned char row, unsigned char col)
{
    
}