#ifndef BOOCH_BOTV2_H
#define BOOCH_BOTV2_H

void Initialize(void);
void Done(void);
void StartMenu(void);
void Brewing(void);
void rotary_encoder_init(void);
void relay_init();

//#ifndef DEFINE_ISR
//ISR(PCINT1_vect);
//#endif

#endif