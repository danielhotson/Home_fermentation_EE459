#include <avr/io.h>
#include <util/delay.h>
#include "relay.h"
/*

*/
void relay_on(int CTRL_PORT)
{
    //DDRD |= (1 << PD2);

    PORTD |= (1 << CTRL_PORT); // set PD2 to a 1

    return;
}

/*
*/
void relay_off(int CTRL_PORT)
{
    //DDRD != (1 << PC2);

    PORTD &= ~(1 << CTRL_PORT);

    return;
}
