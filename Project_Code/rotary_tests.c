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

    PORTB |= ((1 << PCINT2) | (1 << PCINT1)); // set pull-up resistor for PB1 and PB2
    PCICR |= (1 << PCINT0);
    PCMSK0 |= ((1 << PCINT2) | (1 << PCINT1)); // interrupts for PB1 and PB2?
    sei(); // turn global interrupts on?

    lcd_init();
    _delay_ms(100);

    snap = PINB;
    pin_a = (snap & (1 << PCINT1)); // take bit 1
    pin_b = (snap & (1 << PCINT2)); // take bit 2


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

    while(1)
    {

        char temp_string[10];
        sprintf(temp_string, "a: %03d", pin_a);
        lcd_movetoline(0);
        lcd_stringout(temp_string);

        sprintf(temp_string, "b: %03d", pin_b);
        lcd_movetoline(2);
        lcd_stringout(temp_string);

        if (changed)
        {
            changed = 0;

            lcd_movetoline(1);
            sprintf(temp, "%03d", count);
            lcd_stringout(temp);
        }
    }

    return 0;
}

ISR(PCINT0_vect)
{
    

    snap = PINB;
    pin_a = (snap & (1 << PCINT1)); // take bit 1
    pin_b = (snap & (1 << PCINT2)); // take bit 2


    if (old_state == 0)
    {
        if (pin_a != 0){
            new_state = 1;
        }

        else if (pin_b != 0)
        {
            new_state = 2;
        }
    }

    else if (old_state == 1)
    {
        if (pin_a == 0)
        {
            new_state = 0;
        }

        else if (pin_b != 0)
        {
            new_state = 3;
        }

    }

    else if (old_state == 2)
    {
        if (pin_a != 0)
        {
            new_state = 3;
        }
        else if (pin_b == 3)
        {
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