#include "IIKit.h"

void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

void managerInputFunc(void) {
    const uint16_t vlPOT1 = IIKit.analogReadPot1(); //analogRead(def_pin_ADC1);
    const uint16_t vlPOT2 = IIKit.analogReadPot2(); //analogRead(def_pin_ADC2);
    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    wserial::plot("vlPOT1", vlPOT1);
    wserial::plot("vlPOT2", vlPOT2);
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

  static uint64_t previousTimeMS1 = 0;
  if ((currentTimeMS - previousTimeMS1) >= 500)
  {
    previousTimeMS1 = currentTimeMS;
    blinkLEDFunc(def_pin_D1);
  }

  
  static uint64_t previousTimeMS2 = 0;  
  if ((currentTimeMS - previousTimeMS2) >= 50)
  {
    previousTimeMS2 = currentTimeMS;
    managerInputFunc();
  } 
}