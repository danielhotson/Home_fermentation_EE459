//===========================================================================
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "i2c.h"
//----------------------------------------------------------------------------
#define Cword 0x14  //20
#define LINE_ONE    0x80  // DD RAM Address The starting position of the first line is 0x00
                          // set the DD RAM address to 0x80 + 0x00 = 0x80
#define LINE_TWO    0xc0  // DD RAM Address The starting position of the second line is 0x40
                          // set the DD RAM address to 0x80 + 0x40 = 0xc0
#define LINE_THREE  0x94  // DD RAM Address The starting position of the second line is 0x14
                          // set the DD RAM address to 0x80 + 0x14 = 0x94
#define LINE_FOUR   0xD4  // DD RAM Address The starting position of the second line is 0x54
                          // set the DD RAM address to 0x80 + 0x54 = 0xD4
//============================================================================

// Find divisors for the UART0 and I2C baud rates
#define FOSC 7372890            // Clock frequency = Oscillator freq.
#define BAUD 9600               // UART0 baud rate
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define BDIV (FOSC / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz

// LCD SPI & control lines
//   ARD      | Port | LCD
// -----------+------+-------------------------
//  #13/D13   |  PB5 | SD_CS

// Four 7-bit slave addresses (0111100, 0111101, 0111110 and 0111111) are reserved for the RW1063. The least
// significant bit of the slave address is set by connecting the input SA0 (DB0) and SA1 (DB1) to either logic 0
// (VSS) or logic 1 (VDD).
// #define I2C_ADDRESS 0x78 //0b01111000
// #define I2C_ADDRESS 0x7A //0b01111010
// #define I2C_ADDRESS 0x7C //0b01111100
 #define I2C_ADDRESS 0x7E //0b01111110

//                  //00000000001111111111
//                  //01234567890123456789
uint8_t MSG1[20]    ="**    BOOCH-BOT   **";
uint8_t MSG2[20]    ="**     EE459x     **";
uint8_t MSG3_1[20]  ="**  Georgia, Jack **";
uint8_t MSG3_2[20]  ="**   and Daniel   **";
uint8_t MSG4[20]    ="**    Group 12    **";

uint8_t CGRAM[8][8] = {
  {21, 42, 21, 42, 21, 42, 21, 42},
  {42, 21, 42, 21, 42, 21, 42, 21},
  {63, 31, 15,  7,  3,  1,  0,  0},
  { 0,  0, 32, 48, 56, 60, 62, 63},
  { 1,  2,  4,  8,  8,  4,  2,  1},
  {32, 16,  8,  4,  4,  8, 16, 32},
  {62, 61, 59, 55, 47, 31, 47, 55},
  {59, 61, 62, 61, 59, 55, 47, 31},
};
//============================================================================
void writeByte(uint8_t byte);
void writeCommand(uint8_t command);
void writeData(uint8_t data);
void writeString(uint8_t count, uint8_t *MSG);
void Initialize_CFAH2004AC(void);
void Initialize_CGRAM(void);

//============================================================================
void writeByte(uint8_t byte){
  i2c_io(I2C_ADDRESS, &byte, 1, NULL , 0);
}
//============================================================================
void writeCommand(uint8_t command)
{
  //Send the command via I2C:
  writeByte(0x00);
  writeByte(command);

  _delay_ms(1);		// Delay for 1ms
}
//============================================================================
void writeData(uint8_t data)
{
  //Send the data via I2C:
  writeByte(0xC0);		        //Control Byte - Data (non-continued)
  writeByte(data);

  _delay_ms(1);
}
//============================================================================
void writeString(uint8_t count,uint8_t *MSG)
{
  for(uint8_t i = 0; i<count;i++)
  {
    writeData(MSG[i]);
  }
}
//============================================================================
void Initialize_CFAH2004AC()
{
  i2c_init(BDIV);
  
  //LCD Init
  _delay_ms(50);
  writeCommand(0x38); // Function set
  _delay_ms(1);
  writeCommand(0x0C); // Display ON/OFF
  _delay_ms(1);
  writeCommand(0x01); // Clear display
  _delay_ms(10);
  writeCommand(0x06); // Entry mode set
  _delay_ms(1);
}
//============================================================================
void Initialize_CGRAM(void)
{
  
  unsigned char i, j;

  // The first graph of the CGRAM Address start address
  //  is 000000 (0x00) CGRAM address is set to 0x40 + 0x00 = 0x40
  // The second graph's CGRAM Address starts with address 001000 (0x08)
  // etc.
  for (i = 0; i < 8; i ++)
  {
    writeCommand(0x40 + (0x08 * i));
    for (j = 0; j < 8; j ++)
    {
      writeData(CGRAM[i][j]);
    }      
  }
}

int main(void) {

  DDRB |= 0xFF;
  
  //Fire up the I2C LCD
  Initialize_CFAH2004AC();
  Initialize_CGRAM();
  
  //From the init
  writeCommand(LINE_ONE);
	writeString(Cword,MSG1);
	writeCommand(LINE_TWO);
  writeString(Cword,MSG2);
	writeCommand(LINE_THREE);
	writeString(Cword,MSG3_1);
	writeCommand(LINE_FOUR);
	writeString(Cword,MSG4);

  uint8_t currentLine[20] = "** CFAH2004AC-TFH **";
  uint8_t iteration = 0;

  while (1) {
    uint8_t temp;

    //This is the scrolling
    temp = currentLine[0];
    for(uint8_t thisChar = 1; thisChar < 20; thisChar++)
    {
      currentLine[thisChar - 1] = currentLine[thisChar];
    }
    currentLine[19] = temp;
    writeCommand(LINE_TWO);
    writeString(Cword,currentLine);
    iteration++;
    _delay_ms(150);

    //Scrolling is done
    if(iteration == 20)
    {
      _delay_ms(1000);

      //Show off the CGRAM
      writeCommand(LINE_THREE);
      writeString(Cword,MSG3_2);
      
      writeCommand(LINE_FOUR);
      writeData('*');
      writeData('*');
      writeData(' ');
      writeData(' ');
      writeData(' ');
      writeData(' ');
      for(uint8_t thisChar = 0; thisChar < 8; thisChar++)
      {
        writeData(thisChar);
      }
      writeData(' ');
      writeData(' ');
      writeData(' ');
      writeData(' ');
      writeData('*');
      writeData('*');
      _delay_ms(10000);

      //Put the text back
      writeCommand(LINE_THREE);
      writeString(Cword,MSG3_1);
      writeCommand(LINE_FOUR);
      writeString(Cword,MSG4);
      _delay_ms(1000);
      iteration = 0;
    }
   }
}