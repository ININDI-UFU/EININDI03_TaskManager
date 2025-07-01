#include "IIKitmini.h"
#include "util/asyncDelay.h"

AsyncDelay_c blinkLED(500); // time mili second
void blinkLEDFunc(uint8_t pin) {
  if (blinkLED.isExpired()) {
    digitalWrite(pin, !digitalRead(pin));
  }
}

AsyncDelay_c delayPOT(50); // time mili second
void managerInputFunc(void) {
  if (delayPOT.isExpired()) {
    const uint16_t vlPOT1 = analogRead(def_pin_ADC1);
    const uint16_t vlPOT2 = analogRead(def_pin_ADC2);
    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    IIKit.WSerial.plot("vlPOT1", vlPOT1);
    IIKit.WSerial.plot("vlPOT2", vlPOT2);
  }
}

void setup()
{
  IIKit.setup();
  pinMode(def_pin_D1, OUTPUT);
}

void loop()
{
  IIKit.loop();
  blinkLEDFunc(def_pin_D1);
  managerInputFunc();
}