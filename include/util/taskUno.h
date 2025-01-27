#ifndef __TASK_H
#define __TASK_H

#include "Arduino.h"
#include <TimerOne.h> // Certifique-se de ter a biblioteca TimerOne instalada

// Estrutura de configuração
struct CounterConfig
{
    uint16_t counter; // Contador
    uint16_t limit;   // Limite do contador
    void (*task)();   // Função a ser chamada quando o limite é atingido
};

// Esta variável será definida no arquivo principal (ex: no .ino)
CounterConfig *taskStruct = nullptr; // Ponteiro para o array de configurações
size_t taskCount = 0;                // Quantidade de configurações no array

// Parâmetros da "fila"
#define TASK_QUEUE_SIZE 20  // Ajuste conforme a necessidade
volatile void (*taskQueue[TASK_QUEUE_SIZE])();
volatile uint8_t taskQueueHead = 0;
volatile uint8_t taskQueueTail = 0;

// Função chamada pelo timer (ISR)
void timerCallback()
{
    // Incrementa contadores e verifica limites
    for (size_t i = 0; i < taskCount; ++i)
    {
        CounterConfig &c = taskStruct[i];
        if (c.counter >= c.limit)
        {
            // Tenta colocar a função na fila
            uint8_t nextTail = (taskQueueTail + 1) % TASK_QUEUE_SIZE;
            if (nextTail != taskQueueHead) // Verifica se há espaço na fila
            {
                taskQueue[taskQueueTail] = reinterpret_cast<volatile void (*)()>(c.task);
                taskQueueTail = nextTail;
            }
            c.counter = 0; // Reinicia o contador
        }
    }
}

// Configuração inicial do timer
// frequency: frequência em Hz
// Ex: se frequency = 1000, a ISR é chamada 1000 vezes por segundo (a cada 1ms)
void taskSetup(CounterConfig *tasks, size_t count, uint32_t frequency=1000)
{
    taskStruct = tasks; // Passa o ponteiro do array de configurações para a variável global
    taskCount = count;  // Armazena o tamanho do array
    // Calcula o período em microssegundos
    unsigned long periodMicroseconds = 1000000UL / frequency;

    // Inicializa o timer com a frequência desejada
    Timer1.initialize(periodMicroseconds);
    Timer1.attachInterrupt(timerCallback);

    // Inicializa a "fila"
    taskQueueHead = 0;
    taskQueueTail = 0;
}

// Função para executar as tarefas da fila
void taskLoop()
{
    // Enquanto houver itens na fila
    noInterrupts(); // Desabilita interrupções momentaneamente para acesso seguro aos índices
    while (taskQueueHead != taskQueueTail)
    {
        void (*taskMessage)() = reinterpret_cast<void (*)()>(taskQueue[taskQueueHead]);
        taskQueueHead = (taskQueueHead + 1) % TASK_QUEUE_SIZE;
        interrupts(); // Reabilita interrupções para executar a função de forma segura
        // Executa a tarefa
        taskMessage();
        noInterrupts(); // Desabilita novamente para verificar próximo item
    }
    interrupts();
}

#endif
