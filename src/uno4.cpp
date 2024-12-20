#include <Arduino.h>

#define MAXLENGTHJQUEUE 20
#define NUM_TASKS 2
#include "util/jtask.h"

#define PINLED 3

float i = 0;
void escreve_serial(void)
{
    i += 0.1;
    // Print log
    Serial.print("Instante: ");
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

void blinkLEDFunc(uint8_t pin)
{
    digitalWrite(pin, !digitalRead(pin));
}

// Setup normal do Arduino
void setup() {
    Serial.begin(115200);
    jtaskSetup(1000);    // Configura o timer para 1000 Hz (1 ms)
    jtaskAttachFunc(escreve_serial, 50);
    jtaskAttachFunc([]() { blinkLEDFunc(PINLED); }, 500);    
}

// Loop normal do Arduino
void loop() {
    jtaskLoop(); // Executa as tarefas enfileiradas
    // Outras lógicas do seu programa ...
}
