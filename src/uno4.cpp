#include <Arduino.h>
#define MAXLENGTHJQUEUE 1024
#define NUMTASKS 2
#include "util/jtask.h"

#define PINLED 3

float i = 0;

// Função para imprimir dados no Serial
void escreve_serial(void) {
    i += 0.1;
    Serial.print("Instante: ");
    Serial.println(i);

    Serial.print(">sin:");
    Serial.print(i);
    Serial.print(":");
    Serial.print(sin(i));
    Serial.println("|g");

    Serial.print(">Func:");
    Serial.print(i);
    Serial.print(":");
    Serial.print(0.8 * sin(i) + 0.2 * cos(i));
    Serial.println("|g");
}

// Função para piscar LED
void blinkLEDFunc() {
    digitalWrite(PINLED, !digitalRead(PINLED));
}

// Configuração inicial
void setup() {
    Serial.begin(
        
    );    
    pinMode(PINLED, OUTPUT);
    jtaskSetup(1000);    // Configura o timer para 1000 Hz (1 ms)
    jtaskAttachFunc(escreve_serial, 50); //anexa um função e sua base de tempo para ser executada
    jtaskAttachFunc(blinkLEDFunc, 500);  //anexa um função e sua base de tempo para ser executada
}

// Loop principal
void loop() {
    jtaskLoop(); // Executa as tarefas enfileiradas
    // Outras lógicas do programa ...
}
