#include <Arduino.h>
#include "util/asyncDelay.h"
#define PINLED 3

float i = 0;

void escreve_serial(void)
{
    i += 0.1;
    // Print log
    Serial.print("casa");
    Serial.println(i);

    // Plot a sinus
    Serial.print(">sin:");
    Serial.print(i);
    Serial.print(":");
    Serial.print(sin(i));
    Serial.println("|g");

    // Plot a cosinus
    Serial.print(">Sum:");
    Serial.println(0.8 * sin(i) + 0.2 * cos(i));
}


void blinkLEDFunc(uint8_t pin)
{
    digitalWrite(pin, !digitalRead(pin));
}

void setup()
{
  Serial.begin(115200);
  pinMode(PINLED, OUTPUT);
}

AsyncDelay_c delayES(50); // time mili second
AsyncDelay_c blinkLED(500); // time mili second
void loop()
{
  if (blinkLED.isExpired()) blinkLEDFunc(PINLED);
  if (delayES.isExpired()) escreve_serial();
}