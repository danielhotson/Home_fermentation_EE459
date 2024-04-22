#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "lcd.h"

unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
int count = 0;		// Count to display
unsigned char a, b;
char buf[17];
int note;
unsigned char pin;

int main(void) {
    // Initialize DDR and PORT registers and LCD
	//init LCD
	lcd_init();

	//enable pullup resistors for rotery encoder
	PORTB |= (1 << PB1) | (1 << PB2);
	//enable interupts
	sei();
	PCICR |= (1<<PCIE0);
    PCMSK0 |= ((1 << PCINT1) | (1 << PCINT2)); 


    // Write a spash screen to the LCD
    lcd_stringout("Rotery Encoder Test");
    
    // Read the A and B inputs to determine the intial state.
    // In the state number, B is the MSB and A is the LSB.
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.    
	pin = PINC;

	a = pin & (1<<PC1);
	b = pin & (1<<PC5);

    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    new_state = old_state;

    while (1) {                 // Loop forever

		if (changed) { // Did state change?
			changed = 0;        // Reset changed flag

			// Output count to LCD
			snprintf(buf, 17, "%8d", count);
			lcd_moveto(1);
			lcd_stringout(buf);
		}
    }
}
ISR(PCINT1_vect){
	pin = PINC;
	a = pin & (1<<PC1);
	b = pin & (1<<PC5);

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