
/*
  Important NOTES:
    1. If using Arduino IDE, version 1.5.0 or higher is REQUIRED!
*/

/*
  I2cKeypadDemo.ino

  Written by: Gary Muhonen  gary@wht.io

  versions
    1.0.0 - 3/10/2017
      Original release.


  Short Description:

    This demo tests most of the features of the I2cKeypad library.
    The library and demo program work with Arduino and Particle (Photon, Electron, and Core)
    microcontroller boards that are connected to an I2C keypad. The keypad
    connects to the I2C bus using a MCP23008 8 bit interface chip.
    A link below details one such backpack board, and there may others.

    To run this demo program, you must have the serial port on your Particle/Arduino
    connected to a serial monitor program on your computer to see the responses from the keypad.

    See the project details links below for installation and usage information.

    Github repositories:
    * Arduino library files:  https://github.com/wht-io/i2c-keypad-library-arduino.git
    * Particle library files: https://github.com/wht-io/i2c-keypad-library-particle.git

    Project Details:

    * Library installation and usage: http://wht.io/portfolio/i2c-keypad-library/
    * I2C Keypad backpack board: http://wht.io/portfolio/i2c-keypad-backpack-board/

*/

/*
  This demo program is public domain. You may use it for any purpose.
  NO WARRANTY IS IMPLIED.
*/

#ifdef ARDUINO_ARCH_AVR         // if using an arduino
#include <I2cKeypad.h>
#include <Wire.h>
#elif PARTICLE                  // if using a core, photon, or electron (by particle.io)
#include <I2cKeypad.h>
#else                           // if using something else
#endif

#define KEYPAD_ADDRESS     0x20                // i2c address for the keypad
#define KEYPAD_DEBOUNCE_TIME 20                // 20ms debounce time for the keypad
#define KEYPAD_ROWS 4
#define KEYPAD_COlS 4
byte rowPins[KEYPAD_ROWS] = {0, 1, 2, 3};      // these are the i/o pins for each keypad row connected to the MCP23008 chip
byte colPins[KEYPAD_COlS] = {4, 5, 6, 7};      // these are the i/o pins for each keypad column connected to the MCP23008 chip

// This is where you define what character you want returned from the getKey() library function when a key is pressed on the keypad.
// The top row of this array corresponds to the top row on the keypad. You can use any ASCII character 1-255.
char keyMap[KEYPAD_ROWS][KEYPAD_COlS] = {
  {'1','2','3','A'},                                // row 1 keys
  {'4','5','6','B'},                                // row 2 keys
  {'7','8','9','C'},                                // row 3 keys
  {'0','-','.','E'}                                 // row 4 keys
};

// create a keypad object
I2cKeypad keypad( ((char*)keyMap), rowPins, colPins,  KEYPAD_ROWS, KEYPAD_COlS, KEYPAD_DEBOUNCE_TIME, KEYPAD_ADDRESS);

// In order to not miss keypresses, call at least one of the keypad functions approximately every 10ms (or less).
// In this demo program, we are calling one of the keypad functions very often in loop(), so a timer is not necessary.
// If needed, you can configure a timer to automatically call the keypad function scanKeys() every 10ms.
// If you don't want use a timer for this, then you need to call one of the keypad functions in your loop()
//    at least every 10ms, so that keys are scanned for frequently.
// If using a Particle device, we can set up one of the Particle timers to automatically the function scanKeys() every 10ms.
// If using an Arduino, you could use a timer library to do this also.
#ifdef PARTICLE
  // if needed, configure a Particle timer to call the keypad function scanKeys().
  // Timer keypadTimer(10, &I2cKeypad::scanKeys, keypad);
#elif ARDUINO_ARCH_AVR
  // if needed, configure an arduino class timer here to call the keypad function scanKeys().
#else
  // no else case needed here
#endif


void setup()
{
  Wire.begin();                         // initialize i2c
  Serial.begin(9600);                   // initialize the Serial port, needed to check keypad operation
  keypad.begin();                       // initialize the keypad

  delay(3000);
  Serial.println("I2cKeypad Demo Program");
  Serial.println();



  // In order to not miss keypresses, call at least one of the keypad functions approximately every 10ms (or less).
  // In this demo program, we are calling one of the keypad functions very often in loop(), so a timer is not necessary.
  // If needed, you can configure a timer to automatically call the keypad function scanKeys() every 10ms.
  // If you don't want use a timer for this, then you need to call one of the keypad functions in your loop()
  //    at least every 10ms, so that keys are scanned for frequently.
  // If using a Particle device, we can set up one of the Particle timers to automatically the function scanKeys() every 10ms.
  // If using an Arduino, you could use a timer library to do this also.
  #ifdef PARTICLE
    // if needed, start the Particle timer to call the keypad function scanKeys().
    // keypadTimer.start();    // start the timer which calls scanKeys() if using a Particle device
  #elif ARDUINO_ARCH_AVR
    // if needed, start the arduino timer here to call the keypad function scanKeys().
  #else
    // no else case needed here
  #endif

}


void loop()
{


  // If you are NOT using a timer to automatically call the scanKeys() function (as defined above),
  // OR you are NOT calling some other keypad functions often in the loop() function below,
  // THEN you may want to call the scanKeys() function here at the top of the loop() function
  //      so that keypresses are not missed.
  // In this demo program, we are calling other keypad functions often, which will scan for keypresses,
  //   so we do not actually need to call it here... but we will because we aren't doing anything
  //   else that is very time critical.
  // call scanKeys() function to scan for keypresses
  keypad.scanKeys();      // scan for any keys that are pressed on the keypad.


  // Here we begin a number of small demo segments, that test various functions in the keypad library.
  // Each segment will run on it's own, if you want comment out other segments.


  // Testing the getKey() function (which returns the next available key in the buffer, if any)
  // This demo reads keys pressed on the keypad and displays them in ASCII, for 10 seconds.
  // Note that the getKey() function does NOT wait for a key to be pressed. It returns binary 0 if no key is pressed.
  // This demo repeatedly calls getKey() and uses the millis() function to determine when 10 seconds is up.
  uint8_t keyFromKeypad1; // this is the returned ASCII value entered for the key pressed on the keypad. If no key pressed, binary 0 is returned.
  unsigned long currentTime1;  // current time from millis()
  keypad.flushKeys(); // Removes any keys from the keypad buffer. May not be necessary if you don't care if keys were already in the buffer.
  Serial.println();
  Serial.println();
  Serial.println("Testing getKey()");
  Serial.println("For the next 10 seconds, press keys, and they will be displayed...");
  currentTime1 = millis();   // get the current time
  // wait here in while loop for 10 seconds
  while (millis() - currentTime1 < 10000)
  {
    // call the getKey() function. This function also calls scanKeys() each time, so you don't need to call scanKeys().
    keyFromKeypad1 = keypad.getKey();
    if (keyFromKeypad1 != RETURN_NO_KEY_IN_BUFFER)               // if a key is pressed, display it (else we just continue on)
    {
      // display the returned ASCII keypad character.
      Serial.print("This key was pressed: ");
      //Serial.write(keyFromKeypad1);             // This displays the key as an ascii character
      //Serial.println();
      Serial.println(char(keyFromKeypad1));       // This also displays the key as an ascii character
      //Serial.println(keyFromKeypad1);           // This displays the key in decimal
    }
    // If we are running on a Particle processor (Photon, Electron, Core, etc.) then we have to
    // call Particle.process() often so that it can maintain connection with the cloud.
    // Since we could be in this while loop a long time, call Particle.process() to
    // keep the cloud connection alive.
    #ifdef PARTICLE
      Particle.process();            // keep particle happy if we sit in this while loop for a long time
    #endif
  }


  // Testing the peekKey() function (which returns the next available key in the buffer, without removing it from the buffer).
  // This demo continuously displays the number of keys in the buffer and the first key stored in buffer, for 10 seconds.
  // Note that the peekKey() and getKeyCount() functions do NOT wait for a key to be pressed.
  // This demo repeatedly calls getKeyCount() and uses the millis() function to see when 10 seconds is up.
  uint8_t keyCount3 = 0; // this is the current number of keys stored in the private library buffer.
  uint8_t lastKeyCount3 = 0; // this is the previous checked number of keys stored in the buffer
  unsigned long currentTime3;  // current millis() value
  keypad.flushKeys(); // Removes any keys from the keypad buffer. May not be necessary if you don't care if keys were already in the buffer.
  Serial.println();
  Serial.println();
  Serial.println("Testing peekKey() - Peek at the first key in buffer.");
  Serial.println("In the next 10 seconds, press up to 32 keys on the keypad...");
  currentTime3 = millis();   // get the current time
  Serial.print("Number of keys in buffer is ");
  Serial.println(keypad.getKeyCount());
  // wait here for 10 seconds
  while (millis() - currentTime3 < 10000)
  {
    // call the checkKey() function. This function also calls scanKeys() each time, so you don't need to call scanKeys().
    // if the key count is different than last time, then display the new key count and peek at the 1st key stored in the buffer.
    keyCount3 = keypad.getKeyCount();
    if (keyCount3 != lastKeyCount3)
    {
      lastKeyCount3 = keyCount3;
      Serial.print("Number of keys in buffer is ");
      Serial.print(keyCount3);
      Serial.print(".    First key saved in buffer is ");
      //Serial.write(keypad.peekKey());             // This displays the key as an ascii character
      //Serial.println();
      Serial.println(char(keypad.peekKey()));       // This also displays the key as an ascii character
      //Serial.println(keypad.peekKey());           // This displays the key in decimal
      // Since we aren't removing chars from buffer, the first char is always the same.
    }
    // If we are running on a Particle processor (Photon, Electron, Core, etc.) then we have to
    // call Particle.process() often so that it can maintain connection with the cloud.
    // Since we could be in this while loop a long time, call Particle.process() to
    // keep the cloud connection alive.
    #ifdef PARTICLE
      Particle.process();           // keep particle happy if we sit in this while loop for a long time
    #endif
  }


  // Testing the getKeyCount() function (which returns the number of keys saved in the buffer).
  // This demo displays the number of keys in the buffer, for 10 seconds.
  // Note that the getKeyCount() function does NOT wait for a key to be pressed. It just returns the number of keys stored in the buffer.
  // This demo repeatedly calls getKeyCount() and uses the millis() function to see when 10 seconds is up.
  uint8_t keyCount2 = 0; // this is the current number of keys stored in the private library buffer.
  uint8_t lastKeyCount2 = 0; // this is the previous checked number of keys stored in the buffer
  unsigned long currentTime2;  // current millis() value
  keypad.flushKeys(); // Removes any keys from the keypad buffer. May not be necessary if you don't care if keys were already in the buffer.
  Serial.println();
  Serial.println();
  Serial.println("Testing getKeyCount()");
  Serial.println("In the next 10 seconds, press up to 32 keys on the keypad...");
  currentTime2 = millis();   // get the current time
  // wait here for 10 seconds
  while (millis() - currentTime2 < 10000)
  {
    // call the checkKey() function. This function also calls scanKeys() each time, so you don't need to call scanKeys().
    // if the key count is different than last time, then print the new key count.
    keyCount2 = keypad.getKeyCount();
    if (keyCount2 != lastKeyCount2)
    {
      lastKeyCount2 = keyCount2;
      Serial.print("Number of keys in buffer is ");
      Serial.println(keyCount2);
    }
    // If we are running on a Particle processor (Photon, Electron, Core, etc.) then we have to
    // call Particle.process() often so that it can maintain connection with the cloud.
    // Since we could be in this while loop a long time, call Particle.process() to
    // keep the cloud connection alive.
    #ifdef PARTICLE
      Particle.process();           // keep particle happy if we sit in this while loop for a long time
    #endif
  }


  // Testing the getKeyUntil() function (returns when 1 key is pressed, or terminates after timeout period)
  // This function demo returns one ASCII character if a key is pressed, or terminates after 10 seconds.
  uint8_t keyFromKeypad; // this is the returned ASCII value entered for the key pressed on the keypad. If no key pressed, binary 0 is returned.
  keypad.flushKeys(); // Removes any keys from the keypad buffer. May not be necessary if you don't care if keys were already in the buffer.
  Serial.println();
  Serial.println();
  Serial.println("Testing getKeyUntil()");
  Serial.println("Press one key within 10 seconds...");
  // call the getKeyUntil() function. This function also calls scanKeys() many times, so you don't need to call scanKeys().
  // This function waits for one of the termination conditions to occur, and then returns (i.e. this function blocks other code from running until it is done).
  // Note: If running a Particle device, this function getKeyUntil() will automatically call Particle.process() to keep the cloud connection alive.
  keyFromKeypad = keypad.getKeyUntil(10000);
  if (keyFromKeypad != RETURN_NO_KEY_IN_BUFFER)  // if a key is pressed, display it.
  {
    // display the returned ASCII keypad character.
    Serial.print("The returned character is: ");
    Serial.println(char(keyFromKeypad));
  }
  else // else binary 0 was returned, so display timeout message.
  {
    Serial.println("No key was pressed within 10 seconds.");
  }



}
