/*
  Livolo.cpp - Library for Livolo wireless switches.
  Created by Sergey Chernov, October 25, 2013.
  Released into the public domain.
*/

#include "Arduino.h"
#include "livolo.h"

Livolo::Livolo(byte pin)
{
  pinMode(pin, OUTPUT);
  txPin = pin;
  // Serial.begin(9600);
}

// keycodes #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
// real remote IDs: 6400; 19303
// tested "virtual" remote IDs: 10550; 8500; 7400
// other IDs could work too, as long as they do not exceed 16 bit
// known issue: not all 16 bit remote ID are valid
// have not tested other buttons, but as there is dimmer control, some keycodes could be strictly system
// use: sendButton(remoteID, keycode), see example blink.ino;


void Livolo::sendButton(unsigned int remoteID, byte keycode) {
  noInterrupts();
  for (repeat = 0; repeat < 200; repeat++) {
    sendPulse(1); // Start
    high = true; // first pulse is always high

    for (i = 16; i > 0; i--) { // transmit remoteID
      byte txPulse = bitRead(remoteID, i - 1); // read bits from remote ID
      selectPulse(txPulse);
    }

    for (i = 7; i > 0; i--) { // transmit keycode
      byte txPulse = bitRead(keycode, i - 1); // read bits from keycode
      selectPulse(txPulse);
    }
  }
  digitalWrite(txPin, LOW);
  interrupts();
  // Serial.println("Sending done");
}

// build transmit sequence so that every high pulse is followed by low and vice versa

void Livolo::selectPulse(byte inBit) {

  switch (inBit) {
    case 0:
      for (byte ii = 1; ii < 3; ii++) {
        if (high == true) {   // if current pulse should be high, send High Zero
          sendPulse(2);
        } else {              // else send Low Zero
          sendPulse(4);
        }
        high = !high; // invert next pulse
      }
      break;
    case 1:                // if current pulse should be high, send High One
      if (high == true) {
        sendPulse(3);
      } else {             // else send Low One
        sendPulse(5);
      }
      high = !high; // invert next pulse
      break;
  }
}

// transmit pulses
// slightly corrected pulse length, use old (commented out) values if these not working for you

void Livolo::sendPulse(byte txPulse) {

  switch (txPulse) { // transmit pulse
    case 1: // Start
      digitalWrite(txPin, HIGH);
      delayMicroseconds(496); // 500
      //digitalWrite(txPin, LOW);
      break;
    case 2: // "High Zero"
      digitalWrite(txPin, LOW);
      delayMicroseconds(137); // 110 // 188 // 140
      digitalWrite(txPin, HIGH);
      break;
    case 3: // "High One"
      digitalWrite(txPin, LOW);
      delayMicroseconds(288); // 303 // 332
      digitalWrite(txPin, HIGH);
      break;
    case 4: // "Low Zero"
      digitalWrite(txPin, HIGH);
      delayMicroseconds(154); // 110 // 117
      digitalWrite(txPin, LOW);
      break;
    case 5: // "Low One"
      digitalWrite(txPin, HIGH);
      delayMicroseconds(314); // 290 // 277
      digitalWrite(txPin, LOW);
      break;
  }
}
