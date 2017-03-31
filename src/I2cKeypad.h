/*
  Important NOTES:
    1. If using Arduino IDE, version 1.5.0 or higher is REQUIRED!
*/

/*
  I2cKeypad.h

  Written by: Gary Muhonen  gary@wht.io

  versions
    1.0.0 - 3/10/2010
      Original Release.


    Short Description:

    This keypad library works with Arduino and Particle (Photon, Electron, and Core)
    microcontroller boards that are connected to an I2C keypad. The keypad
    connects to the I2C bus using a MCP23008 8 bit interface chip.
    A link below details one such backpack board, and there may others.

    A demo program is available that demonstrates most of the features of this library.

    See the project details links below for installation and usage information.

    Github repositories:
    * Arduino library files:  https://github.com/wht-io/i2c-keypad-library-arduino.git
    * Particle library files: https://github.com/wht-io/i2c-keypad-library-particle.git

    Project Details:

    * Library installation and usage: http://wht.io/portfolio/i2c-keypad-library/
    * I2C Keypad backpack board: http://wht.io/portfolio/i2c-keypad-backpack-board/

*/

/*
  Windy Hill Technology LLC code, firmware, and software is released under the
  MIT License (http://opensource.org/licenses/MIT).

  The MIT License (MIT)

  Copyright (c) 2016 Windy Hill Technology LLC

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/


// only include this entire file if it has not been included already
#ifndef I2CKEYPAD_H
#define I2CKEYPAD_H



#ifdef ARDUINO_ARCH_AVR        // if using an arduino IDE version 1.5 or higher is required
#include <Arduino.h>
#include <Wire.h>
#elif PARTICLE                 // if using a core, photon, or electron (by particle.io)
#include <Particle.h>
#else                          // if using something else
#endif


// value returned by getKey(), peekKey(), getKeyUntil() when there are no keys in the buffer to be returned
#define RETURN_NO_KEY_IN_BUFFER 0


// Constants used in this library code.

// macros for bit manipulation of variables
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// registers in MCP23008 chip
#define MCP_IODIR     0x00    // I/O direction register 1=input, 0=output (power on = 0xff)
#define MCP_IPOL      0x01    // Input polarity register 1=input reversed, 0=input normal (0x00)
#define MCP_GPINTEN   0x02    // Interrupt on change control register 1=gpio pin enabled for interrupt (0x00)
#define MCP_DEFVAL    0x03    // Default Compare register for interrupt change (0x00)
#define MCP_INTCON    0x04    // Interrupt control register 1=pin is compared against DEFVAL register, 0=pin is compared against previous pin value (0x00)
#define MCP_IOCON     0x05    // Configuration register (0x00)
#define MCP_GPPU      0x06    // Pullup resistor configuration register 1=pullup enabled, 0=pullup disabled
#define MCP_INTF      0x07    // Interrupt flag register (read only) 1=an interrupt condition on that pin, 0=no interrupt (0x00)
#define MCP_INTCAP    0x08    // Interrupt capture register contains the contents of GPIO port at time of interrupt (0x00)
#define MCP_GPIO      0x09    // GPIO register is the input value of the port (writing writes to the OLAT register)
#define MCP_OLAT      0x0A    // Output latch register (0x00)

/*
IOCON bits
bit 7   0   Unimplemented: Read as ‘0’.
bit 6   0   Unimplemented: Read as ‘0’.
bit 5   1   SEQOP: Sequential Operation mode bit. 1 = Sequential operation disabled, address pointer does not increment. 0 = Sequential operation enabled, address pointer increments.
bit 4   0   DISSLW: Slew Rate control bit for SDA output. 1= Slewratedisabled. 0= Slewrateenabled.
bit 3   0   HAEN: Hardware Address Enable bit (MCP23S08 only). Address pins are always enabled on MCP23008. 1 = Enables the MCP23S08 address pins. 0 = Disables the MCP23S08 address pins.
bit 2   1   ODR: This bit configures the INT pin as an open-drain output. 1 = Open-drain output (overrides the INTPOL bit). 0 = Active driver output (INTPOL bit sets the polarity).
bit 1   0   INTPOL: This bit sets the polarity of the INT output pin. 1= Active-high. 0= Active-low.
bit 0   0   Unimplemented: Read as ‘0’.

In our code we set this register to 0x24
*/

#define MCP_IOCON_VALUE 0x24         // initial value for the MCP IOCON register.

#define KEYPAD_BUFFER_SIZE 32        // size of the keypad buffer, which stores incoming keypad presses.
                                     // This is also the max buffer size for strings and numbers that we are getting from the keypad (see getKeysUntil, getIntUntil, getFloatUntil)

// keypad scanner states used in the function scanKeypad().
#define WAITING_FOR_NEW_KEY_PRESS 0
#define WAITING_DEBOUNCE_TIME 1
#define WAITING_FOR_NO_KEYS_PRESSED 2

// values returned by various INTERNAL functions (these are NOT useful for user's code)
#define NO_KEYS_PRESSED -1
#define TIMEOUT_PERIOD_EXCEEDED -2
#define MAX_NUM_CHARACTERS_REACHED -3
#define TERMINATOR_CHAR_RECEIVED -4
#define MULTIPLE_KEYS_PRESSED -5



class I2cKeypad {                     // class definition
public:

  // constructor function and public functions

  I2cKeypad(char *keyMap, uint8_t *rowPins, uint8_t *colPins,  uint8_t rowNum, uint8_t colNum, uint16_t debounceTime, uint8_t i2cAddress); // creates a keypad object

  void begin();                       // required to initialize the keypad. Run this in setup()!

  void scanKeys(void);              // scan for keys pressed, and puts them in the keypad buffer. This can be run in the loop() function so you don't miss keys
                                      //    OR use some timer feature to call this function frequently to check for keys (10ms is a good period to use).

  uint8_t getKeyCount(void);             // returns the number of characters in the keypad buffer
  uint8_t peekKey(void);                  // returns the next character in the keypad buffer without removing it from the buffer
  uint8_t getKey(void);                   // returns the next character in the keypad buffer and removes it from the buffer

  uint8_t getKeyUntil(uint16_t timeoutPeriod);      // returns one key from the keypad buffer, or waits up to timeoutPeriod for a keypress to occur.

  void    flushKeys(void);                // flush the keypad buffer (removes all keypresses saved in the buffer).


private:
  // private functions used by this library

  uint8_t mcpReadByte(uint8_t mcpRegister);                       // read a byte of data from the MCP chip from register mcpRegister
  uint8_t mcpReadBit(uint8_t mcpRegister, uint8_t bit);           // read one bit (0-7) from register mcpRegister from the MCP chip
  void mcpWriteByte(uint8_t mcpRegister, uint8_t data);           // write a byte of data to the MCP chip to register mcpRegister
  void mcpWriteBit(uint8_t mcpRegister, uint8_t bit, bool data);  // write one bit of data to bit position (0-7) in register mcpRegister
  int  checkForKeyPress(void);                                    // check for a key press, and return it's value


  // private variables

  char    *_keyMap;               // array for mapping which ascii character is returned for each key on the keypad
  uint8_t *_rowPins;              // array for mapping which pin on the MCP23008 corresponds to each row on the keypad
  uint8_t *_colPins;              // array for mapping which pin on the MCP23008 corresponds to each column on the keypad
  uint8_t _rowNum;                // number of rows on the keypad (start counting at 1)
  uint8_t _colNum;                // number of columns on the keypad (start counting at 1)
  uint16_t _debounceTime;         // amount of time to allow for keypad debounce (in milliseconds)
  uint8_t _i2cAddress;            // the i2c address of the mcp23008 chip

  unsigned char _keyBuffer[KEYPAD_BUFFER_SIZE];   // buffer for storing the keys that are pressed
  uint8_t _keyBufferHead;         // index into keyBuffer array for last character to be read
  uint8_t _keyBufferTail;         // index into keyBuffer array for next character to be read

  unsigned long _lastScanTime;    // time of last keypad scan
  uint8_t _keypadState;           // state of the keypad scanner function
  int _lastKeyPressed;            // last value from checkForKeyPress()

  uint8_t _inputPinsMask;         // high bits in this represent input pins on the MCP (which are used for rows on the keypad)


};


#endif        // this is the end of the #ifndef I2CKEYPAD_H from the beginning of this file
