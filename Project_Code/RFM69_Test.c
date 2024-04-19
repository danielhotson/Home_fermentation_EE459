#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "RFM69-Library-AVR/RFM69.h"

#define NODEID        1    //should always be 1 for a Gateway
#define NETWORKID     100  //the same on all nodes that talk to each other
#define FREQUENCY     RF69_433MHZ
#define ENCRYPTKEY    "boochbot12345678"

int main(void)
{
    lcd_init();
    _delay_ms(100);
    lcd_stringout("Radio Test");

    // initialize RFM69
    rfm69_init(433, NODEID, NETWORKID);
    //setHighPower(1);   // if model number rfm69hw
    setPowerLevel(30); // 0-31; 5dBm to 20 dBm 
    encrypt(ENCRYPTKEY);     // if set has to be 16 bytes. example: "1234567890123456"

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

