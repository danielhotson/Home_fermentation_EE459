#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "i2c.h"
//#include "rotary.h"

#define CHANNEL_B PB1  // R1
#define CHANNEL_A PB2  // R2
#define SWITCH PB0     // for button

volatile unsigned char old_state, new_state, snap, pin_a, pin_b, changed;
volatile int count;

int main(void)
{

    //DDRB &= ~(1 << PB1); // sets PB1 for input
    //DDRB &= ~(1 << PB2); // sets PB2 for input

    PORTB |= ((1 << 2) | (1 << 1)); // set pull-up resistor for PB1 and PB2

    PCMSK0 |= ((1 << 2) | (1 << 2)); // interrupts for PB1 and PB2?
    sei(); // turn global interrupts on?

    lcd_init();
    _delay_ms(100);

    snap = PINB;
    pin_a = (snap & (1<<1)); // take bit 1
    pin_b = (snap & (1<<2)); // take bit 2


    if (!pin_b && !pin_a)
    {
        old_state = 0;
    }
    else if (!pin_b && pin_a)
    {
        old_state = 1;
    }
    else if(pin_b && !pin_a)
    {
        old_state = 2;
    }
    else
    {
        old_state = 3;
    }

    new_state = old_state;

    count = 0;

    char temp[8];
    unsigned char state_init;
    state_init = old_state;

    char stupid_temp[10]; // DEBUGGING

    lcd_movetoline(0);
    lcd_stringout("About to Enter While"); // DEBUGGING, can be removed later
    while(1)
    {
        lcd_movetoline(2); // DEBUGGING these can be removed
        sprintf(stupid_temp, "%d", pin_a);
        lcd_stringout(stupid_temp);

        lcd_movetoline(3);
        sprintf(stupid_temp, "%d", pin_b);
        lcd_stringout(stupid_temp);

        if (changed)
        {
            changed = 0;

            lcd_movetoline(1);
            sprintf(temp, "%02d", count);
            lcd_stringout(temp);
        }
    }

    return 0;
}

ISR(PCINT1_vect)
{
    snap = PINC;
    pin_a = (snap & (1<<1)); // take bit 1
    pin_b = (snap & (1<<2)); // take bit 2

    if (old_state == 0)
    {
        if (pin_a != 0){
            count++;
            new_state = 1;
        }

        else if (pin_b != 0)
        {
            count--;
            new_state = 2;
        }
    }

    else if (old_state == 1)
    {
        if (pin_a == 0)
        {
            count--;
            new_state = 0;
        }

        else if (pin_b != 0)
        {
            count++;
            new_state = 3;
        }

    }

    else if (old_state == 2)
    {
        if (pin_a != 0)
        {
            count--;
            new_state = 3;
        }
        else if (pin_b == 3)
        {
            count++;
            new_state = 0;
        }
    }

    else
    {
        if (pin_b == 0)
        {
            count--;
            new_state = 1;
        }
        else if (pin_a == 0)
        {
            count++;
            new_state = 2;
        }
    }

    if (new_state != old_state)
    {
        changed = 1;
        old_state = new_state;
    }
}