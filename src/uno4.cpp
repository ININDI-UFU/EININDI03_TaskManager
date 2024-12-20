#include "util/task.h"
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


// Defina o array de tarefas extern
CounterConfig taskStruct[] = {
    {0, 50, escreve_serial},
    {0, 500,  []() { blinkLEDFunc(PINLED); }},
    // Termine o array com um limit 0 se quiser ou apenas garanta o tamanho fixo
};

// Setup normal do Arduino
void setup() {
    Serial.begin(115200);
    // Configura o timer para 1000 Hz (1 ms)
    taskSetup(1000);
}

void loop() {
    // Executa as tarefas enfileiradas
    taskLoop();
    // Outras lógicas do seu programa ...
}
