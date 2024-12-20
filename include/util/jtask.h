#include "Arduino.h"
#include "util/jqueue.h"

uint8_t jtaskIndex = 0;

#ifndef NUMTASKS
#define NUMTASKS 2
#endif

// Estrutura para configuração das tarefas
struct CounterConfig_t {
    uint16_t counter;
    uint16_t limit;
    void (*task)();
};

CounterConfig_t jtaskStruct[NUMTASKS]; // Array de tarefas
jQueue_t jtaskQueue; // Fila para execução das tarefas

#ifdef ESP32
hw_timer_t *timer = nullptr;
void IRAM_ATTR timerCallback() { // Função de callback do timer no ESP32
#else
ISR(TIMER2_COMPA_vect) { // Função de callback do timer no Arduino Uno (Timer 2)
#endif    
    for (uint8_t i = 0; i < jtaskIndex; i++) {
        if (++jtaskStruct[i].counter >= jtaskStruct[i].limit) {
            jQueueSendFromISR(&jtaskQueue, jtaskStruct[i].task);
            jtaskStruct[i].counter = 0;
        }
    }
}


// Configuração do timer
bool jtaskSetup(uint32_t frequency) {
    #ifdef ESP32
    timer = timerBegin(0, 80, true); // Inicializa o timer com prescaler de 80
    timerAttachInterrupt(timer, &timerCallback, true); // Configura a interrupção
    timerAlarmWrite(timer, 80000000 / (80 * frequency), true); // Define a frequência
    timerAlarmEnable(timer); // Ativa o alarme
    #else
    cli(); // Desativa interrupções

    // Configurações para Timer 2
    TCCR2A = 0b00000010; // Modo CTC (Clear Timer on Compare Match)
    TCCR2B = 0b00000100; // Prescaler 64
    OCR2A = (16000000 / (64 * frequency)) - 1; // Calcula o valor de comparação
    TIMSK2 = 0b00000010; // Habilita interrupção por comparação A (OCIE2A)

    sei(); // Habilita interrupções globais
    #endif
    return jQueueCreate(&jtaskQueue); // Cria a fila de tarefas
}

// Anexa uma nova tarefa
bool jtaskAttachFunc(void (*task)(), uint16_t limit) {
    if (jtaskIndex >= NUMTASKS) return false; // Verifica limite de tarefas
    jtaskStruct[jtaskIndex] = {0, limit, task}; // Configura a tarefa
    jtaskIndex++;
    return true;
}

// Executa as tarefas enfileiradas
void jtaskLoop() {
    void (*taskMessage)();
    while (jQueueReceive(&jtaskQueue, &taskMessage)) {
        taskMessage(); // Executa a tarefa
    }
}
