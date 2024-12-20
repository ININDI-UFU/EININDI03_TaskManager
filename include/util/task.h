#include "Arduino.h" // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#include "util/jqueue.h"

#ifdef ESP32
    // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
    hw_timer_t *timer = nullptr; // Configuração global do timer
#else
    #include <TimerOne.h>
#endif

    jQueueHandle_t taskQueue;     // Declaração de uma fila do FreeRTOS para armazenar as ações que devem ser feitas

    struct CounterConfig
    {
        uint16_t counter; // Ponteiro para o contador
        uint16_t limit;   // Limite do contador
        void (*task)();
    };

    extern CounterConfig taskStruct[]; // Permite que `counters` seja definido fora da biblioteca


    // Função chamada pelo timer
#ifdef ESP32
    void IRAM_ATTR timerCallback()
#else
    void timerCallback()
#endif    
    {
        for (auto &c : taskStruct)
        {
            if (++c.counter == c.limit)
            {
                jQueueSendFromISR(taskQueue, &c.task); // Envia o valor para a fila
                c.counter = 0;
            }
        }
    }

    // Configuração inicial do timer
    // frequency: frequência em Hz
    // Ex: se frequency = 1000, a ISR é chamada 1000 vezes por segundo (a cada 1ms)      
    void taskSetup(uint32_t frequency)
    {
        // Configuração do timer
        #ifdef ESP32
            timer = timerBegin(frequency);                           // Inicializa o timer com a frequência de amostragem.
            timerAttachInterrupt(timer, &timerCallback);             // Associa a função timerCallback como a ISR do timer.
            timerAlarm(timer, (80000000 / 80) / frequency, true, 0); //  // Set alarm to call onTimer (value in microseconds).
        #else      
            // Calcula o período em microssegundos
            unsigned long periodMicroseconds = 1000000UL / frequency;
            // Inicializa o timer com a frequência desejada
            Timer1.initialize(periodMicroseconds);
            Timer1.attachInterrupt(timerCallback);
        #endif


        // Inicializa a fila com capacidade para NUM_SAMPLES elementos, cada um do tamanho de uint16_t.
        taskQueue = jQueueCreate();
    }

    void taskLoop()
    {
        volatile void (*taskMessage)(); // Declaração de um ponteiro para uma função, que será recebido da fila.
        // Verifica continuamente se há dados na fila
        while (jQueueReceive(taskQueue, &taskMessage) == true)
        {
            // Se xQueueReceive retorna pdTRUE, significa que há um item disponível na fila:
            // - O dado é removido da fila e armazenado em taskMessage.
            taskMessage();
        }
    }
