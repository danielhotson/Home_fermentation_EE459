#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

uint16_t ethanol_read(uint8_t adcx);

uint16_t ethanol_read(uint8_t adcx) {
	ADMUX	&=	0xf0;
	ADMUX	|=	adcx;

	ADCSRA |= _BV(ADSC);

	while ((ADCSRA & _BV(ADSC)));

	return ADC;
}