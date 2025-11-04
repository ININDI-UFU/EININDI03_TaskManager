#include "IIKit.h"

void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

void managerInputFunc(void) {
    const uint16_t vlPOT1 = IIKit.analogReadPot1(); //analogRead(def_pin_ADC1);
    const uint16_t vlPOT2 = IIKit.analogReadPot2(); //analogRead(def_pin_ADC2);
    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    IIKit.WSerial.plot("vlPOT1", vlPOT1);
    IIKit.WSerial.plot("vlPOT2", vlPOT2);
}

void setup()
{
  IIKit.setup();
  pinMode(def_pin_D1, OUTPUT);
}

AsyncDelay_c blinkLED(500); // time mili second
AsyncDelay_c delayPOT(50); // time mili second
void loop()
{
  IIKit.loop();
  if (blinkLED.isExpired()) blinkLEDFunc(def_pin_D1);
  if (delayPOT.isExpired()) managerInputFunc();
}