#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "i2c.h"
//#include "rotary.h"

#define CHANNEL_B PCINT1  // R1
#define CHANNEL_A PCINT2  // R2
#define BUTTON PCINT0     // for button

volatile unsigned char old_state, new_state, snap, pin_a, pin_b, changed;
volatile unsigned char buttonPressed = 0; // 1 if the button is pressed, 0 otherwise
volatile unsigned char buttonChanged = 0;
volatile int count;

int main(void)
{
    DDRB &= ~(1 << BUTTON);
    PORTB |= ((1 << CHANNEL_A) | (1 << CHANNEL_B) | (1 << BUTTON)); // set pull-up resistor for PB1 and PB2
    PCICR |= (1 << BUTTON);
    PCMSK0 |= ((1 << CHANNEL_A) | (1 << CHANNEL_B) | (1 << BUTTON)); // interrupts for PB1 and PB2?
    sei(); // turn global interrupts on?

    lcd_init();
    _delay_ms(100);

    snap = PINB;
    pin_b = (snap & (1 << CHANNEL_B)); // take bit 1
    pin_a = (snap & (1 << CHANNEL_A)); // take bit 2


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

    while(1)
    {

        if (changed || buttonChanged)
        {
            if (changed)
            {
                //lcd_movetoline(1);
                //sprintf(temp, "%03d", count);
                //lcd_stringout(temp);

                /*
                    Add Code Here
                */
                changed = 0;
            }
            if (buttonChanged)
            {
                /*
                    Add Code here
                */
                _delay_ms(50);
                buttonChanged = 0;
            }

            
        }
        _delay_ms(20);
    }

    return 0;
}

ISR(PCINT0_vect)
{
    

    snap = PINB;
    pin_b = (snap & (1 << PCINT1)); // take bit 1
    pin_a = (snap & (1 << PCINT2)); // take bit 2

    if (PINB & (1 << BUTTON))
    {
        if (buttonPressed == 0)
        {
            buttonPressed = 1;
        }
        else{
            buttonPressed = 0;
            buttonChanged = 1;
        }
    }

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
        else if (pin_b == 0)
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