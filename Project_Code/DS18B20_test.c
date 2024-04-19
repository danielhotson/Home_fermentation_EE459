#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "ds18b20.h"
#define OUTBUFSIZE 20


int main(void)
{
    lcd_init();
    ds_init();
    DDRC |= (1 << PC0); //Set PC0 (led) for output
    PORTC |= (1 << PC0); //turn on Port C
    _delay_ms(100);
    lcd_stringout("Temperature Sensor");
    lcd_nextLine();
    lcd_stringout("Test");
    PORTC &= ~(1 << PC0); //turn off Port C
    int c16, f10, old_temp;
    int fint, ffrac;
    unsigned char tdata[2];
    char outbuf[OUTBUFSIZE];
    ds_convert();   
    while (1) {
        PORTC |= (1 << PC0); //turn on Port C
       if(ds_temp(tdata)){
            c16 = ((tdata[1] << 8) + tdata[0]); // Centigrade * 16
            f10 = (c16 * 9) / 8 + 320;  // F * 10
            fint = f10 / 10;
            ffrac = f10 % 10;
            if (ffrac < 0)
            ffrac = -ffrac;
            snprintf(outbuf, OUTBUFSIZE, "Temp:%3d.%1d", fint, ffrac);
            lcd_movetoline(2);
            lcd_stringout(outbuf);
            ds_convert();
       }
        PORTC &= ~(1 << PC0); //turn off Port C
        _delay_ms(100);
        
        
    }

    return 0;   /* never reached */
}




