#include "avrc-ds18b20-onewire/pindef.h"
#include "avrc-ds18b20-onewire/onewire.h"
#include "avrc-ds18b20-onewire/ds18b20.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

// ...

// This is a hacked together interface to pass around a port/pin combination by reference
// Everything in pindef.h compiles to quite inefficient asm currently.
const gpin_t sensorPin = { &PORTC, &PINC, &DDRC, PC2 };



int main(void)
{
    lcd_init();
    _delay_ms(100);

    lcd_stringout("Temperature Sensor");
    lcd_nextLine();

    

    while (1) {
        // Send a reset pulse. If we get a reply, read the sensor
        if (onewire_reset(&sensorPin)) {
        
            // Start a temperature reading (this includes skiprom)
            ds18b20_convert(&sensorPin);
            
            // Wait for measurement to finish (750ms for 12-bit value)
            _delay_ms(750);
            
            // Get the raw 2-byte temperature reading
            int16_t reading = ds18b20_read_single(&sensorPin);
            
            if (reading != kDS18B20_CrcCheckFailed) {
                // Convert to floating point (or keep as a Q12.4 fixed point value)
                float temperature = ((float) reading) / 16;
                char buffer[24];
                sprintf(buffer, "    Temperature: %f c", temperature);
                lcd_clear();
                lcd_movetoline(0);
                lcd_stringout(buffer);

            } else {
                // Handle bad temperature reading CRC 
                // The datasheet suggests to just try reading again
            }
        }
        
		_delay_ms(100);
    }

    return 0;   /* never reached */
}




