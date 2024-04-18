#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

int main(void)
{
    lcd_init();
    _delay_ms(100);

    lcd_stringout("abcdefghijklmnopqrstuvwxyz");
    lcd_nextLine();
    lcd_stringout("1234567890");

    while (1) {
		_delay_ms(100);
    }

    return 0;   /* never reached */
}

