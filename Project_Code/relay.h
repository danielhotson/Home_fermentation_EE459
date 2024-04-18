/*
Controls COM-10924 Relay, tested on Sparkfun PCB
*/

#ifndef RELAY_H
#define RELAY_H

void relay_on(int CTRL_PORT);
void relay_off(int CTRL_PORT);

#endif