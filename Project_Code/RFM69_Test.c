#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "Home_fermentation_EE459/Project_Code/lcd.h"
#include "RFM69-Library-AVR/RFM69.h"

#define NETWORKID 33
#define NODEID     4

int main(void)
{
    lcd_init();
    _delay_ms(100);
    lcd_stringout("Radio Test");

    // initialize RFM69
    rfm69_init(433, NODEID, NETWORKID);
    setHighPower(1);   // if model number rfm69hw
    setPowerLevel(30); // 0-31; 5dBm to 20 dBm 
    const char key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    encrypt(key);     // if set has to be 16 bytes. example: "1234567890123456"

  
    while (1) 
    {
	if(receiveDone())
        {
            _delay_ms(10);
            if(ACKRequested())
	    {
		char ack[0];
                sendACK(ack, 0);
	    }
            unsigned char stringData[16];
            for(uint8_t i=0;i<16;i++) // max 16 digit can be shown in this case
            {
                stringData[i]=DATA[i];
            }
            lcd_clear();
            lcd_stringout(stringData);
        }
    }

    return 0;   /* never reached */
}

