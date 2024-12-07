#include <Arduino.h>

#define PINLED1 13
#define TIME_DELAY_MS1 1000
uint64_t previousTimeMS1 = 0;

#define PINLED2 14
#define TIME_DELAY_MS2 500
uint64_t previousTimeMS2 = 0;

void blinkLEDFunc(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

void setup() {
  //Serial.begin(19200);    
  pinMode(PINLED1, OUTPUT);
  pinMode(PINLED2, OUTPUT);  
}

void loop()
{
  const uint64_t currentTimeMS = millis();
  if ((currentTimeMS - previousTimeMS1) >= TIME_DELAY_MS1)
  {
    previousTimeMS1 = currentTimeMS;
    blinkLEDFunc(PINLED1);
  }
  if ((currentTimeMS - previousTimeMS2) >= TIME_DELAY_MS2)
  {
    previousTimeMS2 = currentTimeMS;
    blinkLEDFunc(PINLED2);
  }  
}