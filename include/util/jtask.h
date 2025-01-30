/**
 * @file jtask.cpp
 * @brief Biblioteca para gerenciamento de tarefas usando timers no Arduino e ESP32.
 *
 * Este arquivo implementa a estrutura para configurar, agendar e executar tarefas
 * de maneira eficiente usando filas e interrupções de timer.
 */

#include "Arduino.h"
#include "util/jqueue.h"

/** 
 * @brief Índice para rastrear o número de tarefas registradas.
 */
uint8_t jtaskIndex = 0;

#ifndef NUMTASKS
/**
 * @brief Número máximo de tarefas que podem ser registradas.
 */
#define NUMTASKS 2
#endif

/**
 * @struct CounterConfig_t
 * @brief Estrutura para configuração das tarefas.
 *
 * @param counter Contador interno da tarefa.
 * @param limit Valor limite para acionar a tarefa.
 * @param task Ponteiro para a função da tarefa.
 */
struct CounterConfig_t {
    uint16_t counter;
    uint16_t limit;
    void (*task)();
};

/**
 * @brief Array que armazena as configurações das tarefas.
 */
CounterConfig_t jtaskStruct[NUMTASKS];

/**
 * @brief Fila para gerenciamento e execução das tarefas.
 */
jQueue_t jtaskQueue;

#ifdef ESP32
/**
 * @brief Ponteiro para o timer no ESP32.
 */
hw_timer_t *timer = nullptr;

/**
 * @brief Função de callback do timer no ESP32.
 */
void IRAM_ATTR timerCallback() {
#else
/**
 * @brief Função de callback do timer no Arduino Uno (Timer 2).
 */
ISR(TIMER2_COMPA_vect) {
#endif    
    for (uint8_t i = 0; i < jtaskIndex; i++) {
        if (++jtaskStruct[i].counter >= jtaskStruct[i].limit) {
            jQueueSendFromISR(&jtaskQueue, (void *)jtaskStruct[i].task);
            jtaskStruct[i].counter = 0;
        }
    }
}

/**
 * @brief Configuração do timer para acionar as tarefas periodicamente.
 *
 * @param frequency Frequência em Hz para o timer.
 * @return true se a configuração foi bem-sucedida, false caso contrário.
 */
bool jtaskSetup(uint32_t frequency) {
    jQueueInit(&jtaskQueue); // Inicializa a fila

    #ifdef ESP32
    timer = timerBegin(frequency); // Inicializa o timer com prescaler de 80
    timerAttachInterrupt(timer, &timerCallback); // Configura a interrupção
    // timerAlarmWrite(timer, 80000000 / (80 * frequency), true); // Define a frequência
    // timerAlarmEnable(timer); // Ativa o alarme
    #else
    cli(); // Desativa interrupções

    // Configurações para Timer 2
    TCCR2A = 0b00000010; // Modo CTC (Clear Timer on Compare Match)
    TCCR2B = 0b00000100; // Prescaler 64
    OCR2A = (16000000 / (64 * frequency)) - 1; // Calcula o valor de comparação
    TIMSK2 = 0b00000010; // Habilita interrupção por comparação A (OCIE2A)

    sei(); // Habilita interrupções globais
    #endif

    return true;
}

/**
 * @brief Registra uma nova tarefa para ser executada periodicamente.
 *
 * @param task Ponteiro para a função da tarefa.
 * @param limit Valor limite para acionar a tarefa.
 * @return true se a tarefa foi registrada com sucesso, false caso contrário.
 */
bool jtaskAttachFunc(void (*task)(), uint16_t limit) {
    if (jtaskIndex >= NUMTASKS) return false; // Verifica limite de tarefas
    jtaskStruct[jtaskIndex] = {0, limit, task}; // Configura a tarefa
    jtaskIndex++;
    return true;
}

/**
 * @brief Executa as tarefas enfileiradas.
 *
 * Esta função deve ser chamada regularmente dentro do loop principal do programa.
 */
void jtaskLoop() {
    void (*taskMessage)();
    while (jQueueReceive(&jtaskQueue, (void **)&taskMessage)) {
        taskMessage(); // Executa a tarefa
    }
}