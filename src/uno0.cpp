#include <Arduino.h>
#define PIN_LED 3

float i = 0;
void escreve_serial(){
  i += 0.1;
  
  // Print log
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

  delay(50);
}

void blinkLed(uint8_t pin){
  digitalWrite(pin,!digitalRead(pin));
}

void setup()
{
  Serial.begin(19200);
  pinMode(PIN_LED, OUTPUT);
}


void loop()
{
  delay(500);
  blinkLed(PIN_LED);
  escreve_serial();
}