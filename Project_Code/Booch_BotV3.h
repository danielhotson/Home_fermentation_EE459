#ifndef BOOCH_BOTV3_H
#define BOOCH_BOTV3_H

void Initialize(void);
void Done(void);
void StartMenu(void);
void Brewing(void);
void rotary_encoder_init(void);
void relay_init();
unsigned char decimal_to_bcd(short decimal);

//#ifndef DEFINE_ISR
//ISR(PCINT1_vect);
//#endif

#endif