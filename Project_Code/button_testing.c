#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "lcd.h"

#define CHANNEL_A PCINT2
#define CHANNEL_B PCINT1
#define BUTTON PCINT0

volatile unsigned char snap; // initial snapshot of PINB
volatile unsigned char buttonPressed = 0; // 1 if the button is pressed, 0 otherwise
volatile unsigned char buttonChanged = 0;

int main(void)
{
    DDRB &= ~(1 << BUTTON);
    PORTB |= (1 << BUTTON);
    PCICR |= (1 << BUTTON);
    PCMSK0 |= (1 << BUTTON);
    sei();

    lcd_init();
    _delay_ms(100);

    //snap = PINB;
    //buttonPressed = (snap & (1 << BUTTON));

    while(1)
    {
        if (buttonChanged)
        {
            // do whatever we wanted
            lcd_movetoline(0);
            lcd_stringout("Pressed!");
            _delay_ms(1000);
            lcd_clear();
            buttonChanged = 0;
        }

        _delay_ms(20); // slight delay to prevent bouncing/other interrupts
    }

    return 0;
}


ISR(PCINT0_vect)
{
    if (PINB & (1 << BUTTON))
    {
        if (buttonPressed == 0)
        {
            buttonPressed = 1;
        }
        
    }
    else
    {
        if (buttonPressed == 1)
        {
            buttonPressed = 0;
            buttonChanged = 1;
        }
        
    }
}