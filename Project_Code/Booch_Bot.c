/*************************************************************
*       Booch_Bot.c main project code for EE 459
*
*       Program loops through a state machine to operate booch_bot system
*
* Revision History
* Date     Author      Description
* 03/28/24 D.Hotson    Initial structure
* 
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <structs.h>
#include <lcd.h>

int main(void)
{
	//Adjust initial conditions once pins are determined
    DDRB = 0xFF; 	// Set all PORTB bits for Output
	DDRC = 0xFF; 	// Set all PORTC bits for Output
	DDRD = 0xFF; 	// Set all PORTD bits for Output
	PORTB = 0x00;   // Set all PORTB bits to 0
	PORTC = 0x00;   // Set all PORTB bits to 0
	PORTD = 0x00;   // Set all PORTB bits to 0
	
	//initialize variables
	volatile int state = 0;
	
	// initialize threshold comparison structures
	Kom_Thresh_Ing = Ingredients_create(2, 4, 1, 1, 2); //based off online recipe in cups
	Kom_Thresh_Val1 = Thresholds_create(0,0,0,0); //fill in values
	Kom_Thresh_Val2 = Thresholds_create(0,0,0,0); //fill in values
	
	//infinite loop
    while (1) {
		if (state == 0){ //Initial state
			//initial lcd screen
			//ask user for drink selection
			//ask user for ingredients added
			//compare ingredients added to threshold
			//ask user to press start
			state = 1;
		} else if (state == 1) { //First fermentation
			//loop for set amount of time
			//sensors read in
			//compare sensors to threshold values
			//update display / trigger environment regulation
			
		} else if (state == 2) { //First acknowledge
			//acknowledge lcd screen
			//ask to remove scoby and add flavor
			//ask user to press start
			//close air valve
			state = 3;
			
		} else if (state == 3) { //second fermentation
			//loop for set amount of time
			//sensors read in
			//compare sensors to threshold values
			//update display / trigger environment regulation
			
		} else if (state == 4) { //Second acknowledge
			//acknowledge lcd screen
			//ask user to terminate fermentation
			state = 0;
		}
	}

    return 0;   /* never reached */
}