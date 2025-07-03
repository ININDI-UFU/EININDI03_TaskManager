#include "IIKitmini.h"
#include "util/asyncDelay.h"

#define TIME_DELAY_MS1 500
uint64_t previousTimeMS1 = 0;

#define TIME_DELAY_MS2 50
uint64_t previousTimeMS2 = 0;

void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

void managerInputFunc(void) {
    const uint16_t vlPOT1 = analogRead(def_pin_ADC1);
    const uint16_t vlPOT2 = analogRead(def_pin_ADC2);
    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    IIKit.WSerial.plot("vlPOT1", vlPOT1);
    IIKit.WSerial.plot("vlPOT2", vlPOT2);
}

void setup()
{
  IIKit.setup();
  pinMode(def_pin_D1, OUTPUT);
}

void loop()
{
  IIKit.loop();
  const uint64_t currentTimeMS = millis();
  if ((currentTimeMS - previousTimeMS1) >= TIME_DELAY_MS1)
  {
    previousTimeMS1 = currentTimeMS;
    blinkLEDFunc(def_pin_D1);
  }
  if ((currentTimeMS - previousTimeMS2) >= TIME_DELAY_MS2)
  {
    previousTimeMS2 = currentTimeMS;
    managerInputFunc();
  } 
}