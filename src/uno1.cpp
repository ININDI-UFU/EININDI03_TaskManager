#include <Arduino.h>
#define PINLED 3

#define TIME_DELAY_MS1 500
uint64_t previousTimeMS1 = 0;

#define TIME_DELAY_MS2 50
uint64_t previousTimeMS2 = 0;

float i = 0;
void escreve_serial(){
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

void blinkLEDFunc(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

void setup() {
  Serial.begin(19200);    
  pinMode(PINLED, OUTPUT);  
}

void loop()
{
  const uint64_t currentTimeMS = millis();
  if ((currentTimeMS - previousTimeMS1) >= TIME_DELAY_MS1)
  {
    previousTimeMS1 = currentTimeMS;
    blinkLEDFunc(PINLED);
  }
  if ((currentTimeMS - previousTimeMS2) >= TIME_DELAY_MS2)
  {
    previousTimeMS2 = currentTimeMS;
    escreve_serial();
  }  
}