#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"

// Find divisors for the UART0 and I2C baud rates
#define FOSC 7372890            // Clock frequency = Oscillator freq.
#define BAUD 9600               // UART0 baud rate
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define BDIV (FOSC / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define I2C_ADDRESS 0x7E //0b01111110

int main(void)
{

	DDRC = 0xFF; 	// Set all PORTC bits for Output
	PORTC = 0x00;   // Set all PORTC bits to 0
	unsigned char hexV[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; //hex values for single bit high

    i2c_init(BDIV);

    PORTC = hexV[0]; //turn LED on during init
	_delay_ms(500);

    unsigned char wbuf[8];
    wbuf[0] = 0x80;
    wbuf[1] = 0x38; //Function Set: 2 lines

    i2c_io(I2C_ADDRESS, wbuf, 2, NULL , 0); 
	_delay_ms(120);

    wbuf[1] = 0x0f; //Display on, cursor on, cursor blinks
    i2c_io(I2C_ADDRESS, wbuf, 2, NULL , 0); 
	_delay_ms(120);

    wbuf[1] = 0x01; //Clear display
    i2c_io(I2C_ADDRESS, wbuf, 2, NULL , 0); 
	_delay_ms(120);

    wbuf[1] = 0x06; //Entry mode: cursor shifts right
    i2c_io(I2C_ADDRESS, wbuf, 2, NULL , 0); 
	_delay_ms(120);
    
    wbuf[0] = 0x40; wbuf[1] = 0x48; wbuf[2] = 0x65; wbuf[3] = 0x6C; wbuf[4] = 0x6C; wbuf[5] = 0x6f;
    i2c_io(I2C_ADDRESS, wbuf, 6, NULL , 0); 
	_delay_ms(120);

	PORTC = 0x00;

    while (1) {
        PORTC = hexV[0];
		_delay_ms(100);
		PORTC = 0x00;
		_delay_ms(100);
    }

    return 0;   /* never reached */
}

