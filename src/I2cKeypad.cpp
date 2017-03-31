/*
  Important NOTES:
    1. If using Arduino IDE, version 1.5.0 or higher is REQUIRED!
*/


/*
  I2cKeypad.cpp

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

    * I2cKeypad Library details, installation and usage: http://wht.io/portfolio/i2c-keypad-library/
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


#ifdef ARDUINO_ARCH_AVR        // if using an arduino
#include <I2cKeypad.h>
#elif PARTICLE                 // if using a core, photon, or electron (by particle.io)
#include <I2cKeypad.h>
#else                          // if using something else
#endif


// class constructor
I2cKeypad::I2cKeypad(char *keyMap, uint8_t *rowPins, uint8_t *colPins,  uint8_t rowNum, uint8_t colNum, uint16_t debounceTime, uint8_t i2cAddress)
{
  // save the provided parameters into private variables
  _keyMap = keyMap;
  _rowPins = rowPins;
  _colPins = colPins;
  _rowNum = rowNum;
  _colNum = colNum;
  _debounceTime = debounceTime;
  _i2cAddress = i2cAddress;

  _keyBufferHead = 0;
  _keyBufferTail = 0;
}


// public functions

// perform setup needed for this library - the user should call this function once during setup()
void I2cKeypad::begin()
{
  // create a mask byte where each row pin will have a high bit in _inputPinsMask (keypad rows are inputs on the MCP23008 chip)
  _inputPinsMask = 0;
  for (uint8_t r = 0; r < _rowNum; ++r)
    bitSet(_inputPinsMask, _rowPins[r]);     // set the bit for each pin that is in the row array

  // set up registers in the MCP230008
  mcpWriteByte(MCP_IODIR, _inputPinsMask);   // write 1's for the bits that are inputs
  mcpWriteByte(MCP_IPOL, 0);                 // don't invert the polarity of any input pins
  mcpWriteByte(MCP_GPINTEN, 0);              // no interrupt pins are enabled
  mcpWriteByte(MCP_DEFVAL, 0);               // we are not defining a default value for interrupts
  mcpWriteByte(MCP_INTCON, 0);               // interrupt control register... not used
  mcpWriteByte(MCP_IOCON, MCP_IOCON_VALUE);  // configuration register... see the I2cKeypad.h file for info on MCP_IOCON_VALUE
  mcpWriteByte(MCP_GPPU, 0xff);              // set all input pins to have pullup resistor
  // mcpWriteByte(MCP_INTF, 0);              // Interrupt Flag is read only... no need to initialize it
  // mcpWriteByte(MCP_INTCAP, 0);            // Interrupt Capture is read only... no need to initialize it
  // mcpWriteByte(MCP_GPIO, 0);              // GPIO is read only... no need to initialize it
  mcpWriteByte(MCP_OLAT, 0);                 // Output Latch: set all the output pins low initially

  _lastScanTime = millis();                  // set the current time
  _keypadState = WAITING_FOR_NEW_KEY_PRESS;  // set our state variable used in scanKeys()

}




// Scans the keypad for valid keys, and puts valid keys into _keyBuffer[] (which is a private buffer used by this library)
// This function is called by most of the functions in this library to check if there are any keys being pressed.
// The user should call this function (or one of the functions that calls this function) often (every 10ms or less), so that keys are not missed.
// This function should be run often in loop(), unless you are using some timer feature that calls this function often (every 10ms is a good period).
// parameters
//    none
// returns
//    nothing
// new key presses are added to the _keyBuffer[] array.
void I2cKeypad::scanKeys(void)
{
  int key;    // value returned from checkForKeyPress()
  // just return if we have not waited debounce time, since the last time we were in this function
  if ((millis() - _lastScanTime) <= _debounceTime)
    return;

  _lastScanTime = millis();     // save _lastScanTime with the current time now

  // this is a state machine, where we jump to a different case depending on what we are waiting for to happen
  switch(_keypadState)
  {
    case WAITING_FOR_NEW_KEY_PRESS:         // this state is waiting for a new keypress to occur
      key = checkForKeyPress();
      // check if we have a valid key pressed, and only 1 key pressed
      if (key >= 0)
      {
        // we have a valid key
        _keypadState = WAITING_DEBOUNCE_TIME;   // go to waiting for debounce time state
      }
      // check if multiple keys are pressed (which we don't allow... it is considered the same as no keys pressed)
      else if (key == MULTIPLE_KEYS_PRESSED)
      {
        _keypadState = WAITING_FOR_NO_KEYS_PRESSED;   // go to waiting for no keys to be pressed, since we have to many keys pressed
      }
      // else we have no keys pressed, so we will stay in this state
      break;
    case WAITING_DEBOUNCE_TIME:           // this state is used after a key was pressed and we are waiting for the debounce time to have passed.
      key = checkForKeyPress();
      // check if the same key is pressed as in the previous state... if so then we have a valid key with enough debounce time
      if (key == _lastKeyPressed)
      {
        // we have a valid key
        _keyBuffer[_keyBufferHead] = key;                   // save key in _keyBuffer[]
        _keyBufferHead = (_keyBufferHead + 1) % KEYPAD_BUFFER_SIZE; // increment the index in the buffer array
        _keypadState = WAITING_FOR_NO_KEYS_PRESSED;   // go to waiting for no keys to be pressed
      }
      // check if multiple keys are pressed
      else if (key == MULTIPLE_KEYS_PRESSED)
      {
        _keypadState = WAITING_FOR_NO_KEYS_PRESSED;   // go to waiting for no keys to be pressed, since we have to many keys pressed
      }
      // check if no keys are pressed
      else if (key == NO_KEYS_PRESSED)
      {
        _keypadState = WAITING_FOR_NEW_KEY_PRESS;   // go to waiting for new keys to be pressed, since we have no keys pressed
      }
      break;
    case WAITING_FOR_NO_KEYS_PRESSED:               // this state is used when we are waiting for the last keypress to be released.
      key = checkForKeyPress();
      // change state only if no keys are pressed
      if (key == NO_KEYS_PRESSED)
      {
        // we have no keys pressed
        _keypadState = WAITING_FOR_NEW_KEY_PRESS;   // go to waiting for debounce time state
      }
      break;
    default:
      break;
  }

  _lastKeyPressed = key;                  // save the key that was last pressed

}


// return the number of keys in the keypad buffer
// parameters
//    none
// returns
//    0 if there is no keys in the buffer
//    else returns the number of keys available to read from the buffer
uint8_t I2cKeypad::getKeyCount(void)
{
  scanKeys();             // scan keypad for more keys to be pressed
  return (KEYPAD_BUFFER_SIZE + _keyBufferHead - _keyBufferTail) % KEYPAD_BUFFER_SIZE;
}


// peek at the key in the keypad buffer, but don't remove it from the buffer (you can still run getKey() to retreive it)
// parameters
//    none
// returns
//    RETURN_NO_KEY_IN_BUFFER if the keypad buffer is empty (no keys pressed)
//    the ASCII value of the next key (from the keyMap array)
uint8_t I2cKeypad::peekKey(void)
{
  scanKeys();             // scan keypad for more keys to be pressed
  if (_keyBufferHead == _keyBufferTail) {
    return RETURN_NO_KEY_IN_BUFFER;
  } else {
    return _keyBuffer[_keyBufferTail];
  }
}

// get the next key from the keypad buffer
// parameters
//    none
// returns
//     RETURN_NO_KEY_IN_BUFFER if there is no key in the buffer
//     the ASCII value of the next key pressed (from the keyMap array)
uint8_t I2cKeypad::getKey(void)
{
  scanKeys();             // scan keypad for more keys to be pressed
  // if the head isn't ahead of the tail, we don't have any characters
  if (_keyBufferHead == _keyBufferTail) {
    return RETURN_NO_KEY_IN_BUFFER;
  } else {
    uint8_t c = _keyBuffer[_keyBufferTail];
    _keyBufferTail = (_keyBufferTail + 1) % KEYPAD_BUFFER_SIZE;
    return c;
  }
}

// flush out the _keyBuffer  (removes all previous unread keys from the keypad buffer)
// parameters
//    none
// returns
//    nothing
void I2cKeypad::flushKeys(void)
{
  scanKeys();             // scan keypad for more keys to be pressed
  _keyBufferTail = _keyBufferHead;  // empty the buffer
}




// get one keypad character or wait UNTIL one key is pressed OR we timeout
// parameters
//    timeoutPeriod - amount of time we will wait for one keypad character in milliseconds. If 0, then we
//                      will wait forever for a key press
// returns
//    the ASCII value of the key pressed (from the keyMap array)
//    RETURN_NO_KEY_IN_BUFFER if terminated by timeout (no key in buffer)
uint8_t I2cKeypad::getKeyUntil(uint16_t timeoutPeriod)
{
  unsigned long currentTime;    // save the current time
  uint8_t key;                      // key read from keypad

  currentTime = millis();       // get the current time so we can check for a timeout condition
  // keep waiting for more keypad presses, until a termination contidion occurs
  while(1) {
    // If we are running on a Particle processor (Photon, Electron, Core, etc.) then we have to
    // call Particle.process() often so that it can maintain connection with the cloud. This keypad function
    // could take a long time to finish if no keys are pressed on the keypad.
    #ifdef PARTICLE
      Particle.process();           // keep particle happy if we block for a long time
    #endif

    // check if the timeout feature is enabled (i.e. timeoutPeriod > 0) and if we have timed out
    if (timeoutPeriod && (millis() - currentTime) >= timeoutPeriod)
    {
      return RETURN_NO_KEY_IN_BUFFER;                       // return RETURN_NO_KEY_IN_BUFFER of we timed out before key was pressed.
    }
    key = getKey(); // read a key from the keypad buffer, this function also calls scanKeys() to check for more keypresses
    // if there aren't any chars in the keypad buffer, then just continue to try again
    if (key == RETURN_NO_KEY_IN_BUFFER)
    {
      continue;                 // no keys to read
    }
    return key;   // return the ASCII keypad value
  }
}






// private functions ********************************

// check keypad for key presses
// parameters:
//    none
// return:
//    NO_KEYS_PRESSED (-1)  if no keys pressed
//    MULTIPLE_KEYS_PRESSED (-2)  if multiple keys pressed
//    else if there is a valid key it returns the value from _keyMap[] for the key that is pressed (0-255)
//  Note: This code expects that the IODIR register to have the value of _inputPinsMask (which is all 1s for inputs and 0s for the other bits).
//          and the OLAT register to be set to 0 (all 0s on the output pins).
//          This so so that we can quickly check of any key presses with the first line of code in this function.
int I2cKeypad::checkForKeyPress(void)
{
  bool keyPressed = 0;      // set if a key is pressed
  int  key = NO_KEYS_PRESSED;            // returned key value
  uint8_t outputLatch;   // value to write to the mcp output latch
  uint8_t inputPort;     // value read from the mcp input pins

  // we do a quick check here to see if any keys are pressed... that way we don't have to do the more complicated check below
  //     in the case where no keys are pressed (which is most of the time).
  // If any GPIO pins (that are configured as inputs) are low, then we must have some switch pressed. If they are all high, then no key is pressed, and we just return.
  //    To test this, we see if this is true (meaning no keys pressed):    !(GPIO port & _inputPinsMask) ^ _inputPinsMask
  //    We read the input port, AND it with  _inputPinsMask and then XOR the result with _inputPinsMask (which checks if any of the inputs pins are not high)
  //    After the ^ XOR operation the result will be non-zero if one of the input pins does not match the mask (meaning some key is pressed). We negate it (using !) to get a 0 result if no key pressed.
  // we do not have a key pressed if the follwing is true (note the negation operator)
  if ( !((mcpReadByte(MCP_GPIO) & _inputPinsMask) ^ _inputPinsMask)   )
    return NO_KEYS_PRESSED;

  // for each column, check if any row pins are low
  for (uint8_t col = 0; col < _colNum; ++col)
  {
    outputLatch = 0xff;
    bitClear(outputLatch, _colPins[col]);      // clear the bit for this column output pin
    mcpWriteByte(MCP_IODIR, outputLatch);      // write to the MCP IODIR register to make this one pin an output
                                               //     We make only one pin be an output so that if multiple keys
                                               //     are pressed we don't get two output pins shorted together
                                               //     (and they could be at different levels)
    mcpWriteByte(MCP_OLAT, outputLatch);       // write to the MCP output latch to make the one output pin low
    delayMicroseconds(10);                     // wait for the latch signal to propagate to the input pins
    inputPort = mcpReadByte(MCP_GPIO);         // read the input port
    // for each row pin, check if the pin is low, meaning a key is pressed for this column/row
    for (uint8_t row = 0; row < _rowNum; ++row)
    {
      // check if this row pin is low
      if (bitRead(inputPort, _rowPins[row]) == 0)
      {
        // we have a key press at this row and col
        // check if some other key has already been detected... if so return MULTIPLE_KEYS_PRESSED.
        if (keyPressed)
        {
          // Return the MCP to it's quick key checking state, so that when we come back to the function we can quickly check for a key press
          // Set all input pins to be inputs, and the rest to be outputs...
          mcpWriteByte(MCP_IODIR, _inputPinsMask);   // set all input pins to be inputs, and the rest to be outputs
          // write all output pins to LOW so we can check if any of the input pins are not low later on (meaning a key is pressed)
          mcpWriteByte(MCP_OLAT, 0);
          return MULTIPLE_KEYS_PRESSED;
        }
        // else we have a new key being pressed
        else
        {
          keyPressed = 1;
          key = _keyMap[row * _colNum + col];                // get the key out of the
        }
      }
    }
  }
  // Return the MCP to it's quick key checking state, so that when we come back to the function we can quickly check for a key press
  // Set all input pins to be inputs, and the rest to be outputs...
  mcpWriteByte(MCP_IODIR, _inputPinsMask);   // set all input pins to be inputs, and the rest to be outputs
  // write all output pins to LOW so we can check if any of the input pins are not low later on (meaning a key is pressed)
  mcpWriteByte(MCP_OLAT, 0);
  if (keyPressed)
    return key;
  else
    return NO_KEYS_PRESSED;

}


// read one byte from mcp chip register
// parameters
//    mcpRegister - the register in the mcp chip that is to be read
// returns
//    the the value of the mcp register
uint8_t I2cKeypad::mcpReadByte(uint8_t mcpRegister)
{
  Wire.beginTransmission(_i2cAddress);
  Wire.write(mcpRegister);
  Wire.endTransmission();
  Wire.requestFrom((int)_i2cAddress, (int)1);
  return Wire.read();
}

// write a byte to mcp register
// parameters
//    mcpRegister - the register in the mcp chip that is to be written to
//    data - the value to be written to the mcp register
// returns
//    nothing
void I2cKeypad::mcpWriteByte(uint8_t mcpRegister, uint8_t data)
{
  Wire.beginTransmission(_i2cAddress);
  Wire.write(mcpRegister);
  Wire.write(data);
  Wire.endTransmission();
}

// set mcp register bit (0-7) with data (0-1)
// parameters
//    mcpRegister - the register in the mcp chip that is to be written to
//    bit - which bit of the register that is to be written (0-7)
//    data - the value to be written to the mcp register bit
// returns
//    nothing
void I2cKeypad::mcpWriteBit(uint8_t mcpRegister, uint8_t bit, bool data)
{
  uint8_t mcpRegisterValue;    // current value of the mcp register
  if (bit > 7)
    return;       // we only have 8 pins, 0-7
  mcpRegisterValue = mcpReadByte(mcpRegister);  // read the current value of the register
  if (data == 1)
    mcpRegisterValue |= 1 << bit;     // set the bit if data==1
  else
    mcpRegisterValue &= ~(1 << bit);  // clear the bit if data==0
  mcpWriteByte(mcpRegister, mcpRegisterValue);  // rewrite the register
}

// read one bit from mcp register (returns 0 or 1)
// parameters
//    mcpRegister - the register in the mcp chip that is to be read
//    bit - the bit of the register to be read
// returns
//    the the value of the mcp register bit (0 or 1)
uint8_t I2cKeypad::mcpReadBit(uint8_t mcpRegister, uint8_t bit)
{
  if (bit > 7)
    return 0;       // we only have 8 bits, 0-7
  return (mcpReadByte(mcpRegister) >> bit) & 0x01;       // read register, shift to get just desired bit in 0 position, mask it off with 0x01
}
