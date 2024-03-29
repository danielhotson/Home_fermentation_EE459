//===========================================================================
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
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

#define LCD_CS 0x20
#define CLR_CS    (PORTB &= ~(LCD_CS))
#define SET_CS    (PORTB |=  (LCD_CS))

// Four 7-bit slave addresses (0111100, 0111101, 0111110 and 0111111) are reserved for the RW1063. The least
// significant bit of the slave address is set by connecting the input SA0 (DB0) and SA1 (DB1) to either logic 0
// (VSS) or logic 1 (VDD).
// #define I2C_ADDRESS 0x3C //0b0111100
// #define I2C_ADDRESS 0x3D //0b0111101
// #define I2C_ADDRESS 0x3E //0b0111110
 #define I2C_ADDRESS 0x3F //0b0111111

//                  //00000000001111111111
//                  //01234567890123456789
uint8_t MSG1[20]    ="**  CRYSTALFONTZ  **";
uint8_t MSG2[20]    ="** CFAH2004AC-TFH **";
uint8_t MSG3_1[20]  ="**  20 Characters **";
uint8_t MSG3_2[20]  ="** 8  CUSTOM CHAR **";
uint8_t MSG4[20]    ="**   by 4 lines   **";

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

uint8_t i2c_io(uint8_t, uint8_t *, uint16_t, uint8_t *, uint16_t);
void i2c_init(unsigned char);

//============================================================================
void writeByte(uint8_t byte){
  i2c_io(I2C_ADDRESS, &byte, 1, NULL , 0);
}
//============================================================================
void writeCommand(uint8_t command)
{
  //Assert the CS
  CLR_CS;

  //Send the command via I2C:
  writeByte(0x00);
  writeByte(command);

  //Deassert the CS
  SET_CS;

    _delay_ms(1);		// Delay for 1ms
}
//============================================================================
void writeData(uint8_t data)
{
  //Assert the CS
  CLR_CS;

  //Send the data via I2C:
  writeByte(0xC0);		        //Control Byte - Data (non-continued)
  writeByte(data);
  
  //Deassert the CS
  SET_CS;

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

//============================================================================
/*
  i2c_io - write and read bytes to an I2C device

  Usage:      status = i2c_io(device_addr, wp, wn, rp, rn);
  Arguments:  device_addr - This is the EIGHT-BIT I2C device bus address.
              Some datasheets specify a seven bit address.  This argument
              is the seven-bit address shifted left one place with a zero
              in the LSB.  This is also sometimes referred to as the
              address for writing to the device.
              wp, rp - Pointers to two buffers containing data to be
              written (wp), or to receive data that is read (rp).
              wn, rn - Number of bytes to write or read to/from the
              corresponding buffers.

  This funtions writes "wn" bytes from array "wp" to I2C device at
  bus address "device_addr".  It then reads "rn" bytes from the same device
  to array "rp".

  Return values (might not be a complete list):
        0    - Success
        0x20 - NAK received after sending device address for writing
        0x30 - NAK received after sending data
        0x38 - Arbitration lost with address or data
        0x48 - NAK received after sending device address for reading

  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

This "i2c_io" I2C routine is an attempt to provide an I/O function for both
reading and writing, rather than have separate functions.

I2C writes consist of sending a stream of bytes to the slave device.  In some
cases the first few bytes may be the internal address in the device, and then
the data to be stored follows.  For example, EEPROMs like the 24LC256 require a
two-byte address to precede the data.  The DS1307 RTC requires a one-byte
address.

I2C reads often consist of first writing one or two bytes of internal address
data to the device and then reading back a stream of bytes starting from that
address.  Some devices appear to claim that that reads can be done without
first doing the address writes, but so far I haven't been able to get any to
work that way.

This function does writing and reading by using pointers to two arrays
"wp", and "rp".  The function performs the following actions in this order:
    If "wn" is greater then zero, then "wn" bytes are written from array "wp"
    If "rn" is greater then zero, then "rn" byte are read into array "rp"
Either of the "wn" or "rn" can be zero.

A typical write with a 2-byte address and 50 data bytes is done with

    i2c_io(0xA0, wbuf, 52, NULL, 0);

A typical read of 20 bytes with a 1-byte address is done with

    i2c_io(0xD0, wbuf, 1, rbuf, 20);
*/

uint8_t i2c_io(uint8_t device_addr,
               uint8_t *wp, uint16_t wn, uint8_t *rp, uint16_t rn)
{
    uint8_t status, send_stop, wrote, start_stat;

    status = 0;
    wrote = 0;
    send_stop = 0;

    if (wn > 0) {
        wrote = 1;
        send_stop = 1;

        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);  // Send start condition
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x08)                 // Check that START was sent OK
            return(status);

        TWDR = device_addr & 0xfe;          // Load device address and R/W = 0;
        TWCR = (1 << TWINT) | (1 << TWEN);  // Start transmission
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x18) {               // Check that SLA+W was sent OK
            if (status == 0x20)             // Check for NAK
                goto nakstop;               // Send STOP condition
            return(status);                 // Otherwise just return the status
        }

        // Write "wn" data bytes to the slave device
        while (wn-- > 0) {
            TWDR = *wp++;                   // Put next data byte in TWDR
            TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
            while (!(TWCR & (1 << TWINT))); // Wait for TWINT to be set
            status = TWSR & 0xf8;
            if (status != 0x28) {           // Check that data was sent OK
                if (status == 0x30)         // Check for NAK
                    goto nakstop;           // Send STOP condition
                return(status);             // Otherwise just return the status
            }
        }

        status = 0;                         // Set status value to successful
    }

    if (rn > 0) {
        send_stop = 1;

        // Set the status value to check for depending on whether this is a
        // START or repeated START
        start_stat = (wrote) ? 0x10 : 0x08;

        // Put TWI into Master Receive mode by sending a START, which could
        // be a repeated START condition if we just finished writing.
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
                                            // Send start (or repeated start) condition
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != start_stat)           // Check that START or repeated START sent OK
            return(status);

        TWDR = device_addr  | 0x01;         // Load device address and R/W = 1;
        TWCR = (1 << TWINT) | (1 << TWEN);  // Send address+r
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x40) {               // Check that SLA+R was sent OK
            if (status == 0x48)             // Check for NAK
                goto nakstop;
            return(status);
        }

        // Read all but the last of n bytes from the slave device in this loop
        rn--;
        while (rn-- > 0) {
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // Read byte and send ACK
            while (!(TWCR & (1 << TWINT))); // Wait for TWINT to be set
            status = TWSR & 0xf8;
            if (status != 0x50)             // Check that data received OK
                return(status);
            *rp++ = TWDR;                   // Read the data
        }

        // Read the last byte
        TWCR = (1 << TWINT) | (1 << TWEN);  // Read last byte with NOT ACK sent
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x58)                 // Check that data received OK
            return(status);
        *rp++ = TWDR;                       // Read the data

        status = 0;                         // Set status value to successful
    }
    
nakstop:                                    // Come here to send STOP after a NAK
    if (send_stop)
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);  // Send STOP condition

    return(status);
}


/*
  i2c_init - Initialize the I2C port
*/
void i2c_init(uint8_t bdiv)
{
    TWSR = 0;                           // Set prescalar for 1
    TWBR = bdiv;                        // Set bit rate register
}