#include "Arduino.h" // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#include "util/jqueue.h" // Inclusão de uma biblioteca para gerenciamento de filas.

uint8_t jtaskIndex = 0; // Índice usado para gerenciar as tarefas adicionadas à estrutura de tarefas.

#ifndef NUM_TASKS
#define NUM_TASKS 2 // Define o número máximo de tarefas se não estiver definido anteriormente.
#endif

#ifdef ESP32
    // Configuração específica para ESP32
    // Referência: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
    hw_timer_t *timer = nullptr; // Ponteiro global para o timer de hardware do ESP32.
#else
    // Configuração específica para Arduino Uno sem a biblioteca TimerOne
#endif

// Estrutura para armazenar informações de contadores e limites
struct CounterConfig_t
{
    uint16_t counter; // Contador atual.
    uint16_t limit;   // Limite que, ao ser atingido, dispara a tarefa.
    void (*task)();   // Ponteiro para a função que será executada.
};

// Array para armazenar a configuração de tarefas
CounterConfig_t jtaskStruct[NUM_TASKS]; // Array de tarefas com tamanho definido por NUM_TASKS.

jQueue_t jtaskQueue; // Fila para armazenar as tarefas pendentes a serem executadas.

// Função de callback chamada pelo timer
#ifdef ESP32
    void IRAM_ATTR timerCallback() // No ESP32, essa função precisa ser executada na IRAM (memória de interrupção).
#else
    ISR(TIMER1_COMPA_vect) // No Arduino Uno, a função é chamada diretamente pela interrupção do timer sem TimerOne.
#endif    
{
    // Itera sobre todas as tarefas configuradas
    for (CounterConfig_t *c = jtaskStruct; c->task != nullptr; ++c)
    {
        if (++c->counter == c->limit) // Incrementa o contador e verifica se atingiu o limite.
        {
            jQueueSendFromISR(&jtaskQueue, &c->task); // Adiciona a tarefa na fila para ser executada posteriormente.
            c->counter = 0; // Reinicia o contador.
        }
    }
}

// Configuração inicial do timer
// frequency: frequência em Hz
// Exemplo: se frequency = 1000, a interrupção será chamada 1000 vezes por segundo (1 ms).
bool jtaskSetup(uint32_t frequency)
{
    // Configuração do timer baseada na plataforma
    #ifdef ESP32
        timer = timerBegin(frequency);                           // Inicializa o timer com a frequência desejada.
        timerAttachInterrupt(timer, &timerCallback);             // Associa a função callback ao timer.
        timerAlarm(timer, (80000000 / 80) / frequency, true, 0); // Configura o alarme do timer com base na frequência.
    #else
        cli(); // Desativa interrupções para configurar o timer de forma segura.
        TCCR1A = 0; // Modo normal de operação.
        TCCR1B = 0b00001011; // Configura o prescaler como 64.
        OCR1A = (16000000 / (64 * frequency)) - 1; // Define o valor de comparação com base na frequência desejada.
        TIMSK1 = 0b00000010; // Habilita a interrupção por comparação (OCIE1A).
        sei(); // Habilita interrupções globais.
    #endif

    // Cria a fila para armazenar tarefas pendentes.
    return jQueueCreate(&jtaskQueue);
}

// Adiciona uma nova tarefa à estrutura de tarefas.
bool jtaskAttachFunc(void (*task)(), uint16_t limit) {
    if (jtaskIndex >= NUM_TASKS) { // Verifica se há espaço para adicionar mais tarefas.
        return false; // Retorna false se o número máximo de tarefas for atingido.
    }
    jtaskStruct[jtaskIndex].counter = 0; // Inicializa o contador da tarefa.
    jtaskStruct[jtaskIndex].limit = limit; // Define o limite da tarefa.
    jtaskStruct[jtaskIndex].task = task; // Associa a função da tarefa.
    jtaskIndex++; // Incrementa o índice para a próxima tarefa.
    return true; // Retorna true indicando que a tarefa foi adicionada com sucesso.
}

// Executa tarefas pendentes da fila.
void jtaskLoop()
{
    volatile void (*taskMessage)(); // Ponteiro para armazenar a próxima tarefa a ser executada.
    // Enquanto houver tarefas na fila, executa-as.
    while (jQueueReceive(&jtaskQueue, &taskMessage) == true)
    {
        taskMessage(); // Executa a tarefa recebida da fila.
    }
}
