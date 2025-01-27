#include <Arduino.h>
#include "util/asyncDelay.h"
#define PINLED 3

float i = 0;
AsyncDelay_c delayES(50); // time mili second
void escreve_serial(void)
{
  if (delayES.isExpired()){
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
    Serial.print(i);
    Serial.print(":");
    Serial.print(0.8 * sin(i) + 0.2 * cos(i));    
    Serial.println("|g");
  }
}

AsyncDelay_c blinkLED(500); // time mili second
void blinkLEDFunc(uint8_t pin)
{
  if (blinkLED.isExpired())
  {
    digitalWrite(pin, !digitalRead(pin));
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(PINLED, OUTPUT);
}

void loop()
{
  blinkLEDFunc(PINLED);
  escreve_serial();
}