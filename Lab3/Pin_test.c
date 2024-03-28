/*************************************************************
*       at328-0.c - Demonstrate simple I/O functions of ATmega328
*
*       Program loops turning PC0 on and off as fast as possible.
*
* The program should generate code in the loop consisting of
*   LOOP:   SBI  PORTC,0        (2 cycles)
*           CBI  PORTC,0        (2 cycles)
*           RJMP LOOP           (2 cycles)
*
* PC0 will be low for 4 / XTAL freq
* PC0 will be high for 2 / XTAL freq
* A 9.8304MHz clock gives a loop period of about 600 nanoseconds.
*
* Revision History
* Date     Author      Description
* 09/14/12 A. Weber    Initial Release
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = 0xFF; 	// Set all PORTB bits for Output
	DDRC = 0xFF; 	// Set all PORTC bits for Output
	DDRD = 0xFF; 	// Set all PORTD bits for Output
	PORTB = 0x00;   // Set all PORTB bits to 0
	PORTC = 0x00;   // Set all PORTB bits to 0
	PORTD = 0x00;   // Set all PORTB bits to 0
	unsigned char hexV[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; //hex values for single bit high
	int i;
	
	//infinite loop
    while (1) {
		for(i=0; i<=23; i++){
			if(i<8){
				PORTB = hexV[i];
				_delay_ms(100);
				PORTB = 0x00;
				_delay_ms(100);
			} else if(i<16){
				PORTC = hexV[i-8];
				_delay_ms(100);
				PORTC = 0x00;
				_delay_ms(100);
			} else {
				PORTD = hexV[i-16];
				_delay_ms(100);
				PORTD = 0x00;
				_delay_ms(100);
			}
		}
	}

    return 0;   /* never reached */
}