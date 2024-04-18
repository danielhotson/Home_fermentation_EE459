
#include <avr/io.h>
#include <util/delay.h>
#include "relay.h"

#define CTRL_PIN PD2

/*
Turn relay on for 3 seconds, and then back off
Note: make sure relay.o is included in the Makefile
Note: this is designed for PD2 being the control port connection
*/


int main(void)
{
    DDRD |= (1 << CTRL_PIN);

    relay_on(CTRL_PIN);
    _delay_ms(3000);
    relay_off(CTRL_PIN);

    while(1){
        // we don't want the program to terminate, so just keep it running forever
    }

    return 0;
}