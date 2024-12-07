//https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
#include "IIKit.h"   // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.

#define NUM_SAMPLES 1024    // Define o tamanho máximo da fila (número de amostras armazenadas).
#define SAMPLE_RATE 1000    // Define a taxa de amostragem, neste caso, 1000 amostras por segundo.

hw_timer_t *timer = nullptr; // Configuração global do timer
QueueHandle_t taskQueue; // Declaração de uma fila do FreeRTOS para armazenar as ações que devem ser feitas

void blinkLEDFunc(uint8_t pin);
void managerInputFunc(void);

struct CounterConfig {
    uint16_t counter; // Ponteiro para o contador
    uint16_t limit;   // Limite do contador
    void (*task)();
};
CounterConfig taskStruct[] = {
    {0, 500, [](){blinkLEDFunc(def_pin_D1);}},
    {0, 50, managerInputFunc}
};

// Função chamada pelo timer
void IRAM_ATTR timerCallback() { 
    for (auto &c : taskStruct) {
        if (++c.counter == c.limit) {
            xQueueSendFromISR(taskQueue, &c.task, nullptr); // Envia o valor para a fila
            c.counter = 0;
        }
    }
}

// Configuração inicial do programa
void setup() {
    // Faz as configuções do hardware ESP + Perifericos
    IIKit.setup();
    pinMode(def_pin_D1, OUTPUT);
    // Configuração do timer
    timer = timerBegin(SAMPLE_RATE); // Inicializa o timer com a frequência de amostragem.
    timerAttachInterrupt(timer, &timerCallback); // Associa a função timerCallback como a ISR do timer.
    timerAlarm(timer,  (80000000 / 80) / SAMPLE_RATE, true, 0);//  // Set alarm to call onTimer (value in microseconds).

    // Inicializa a fila com capacidade para NUM_SAMPLES elementos, cada um do tamanho de uint16_t.
    taskQueue = xQueueCreate(NUM_SAMPLES, sizeof(uint16_t));
    if (taskQueue == nullptr) { // Verifica se a criação da fila falhou.
        Serial.println("Erro ao criar a fila!"); // Exibe uma mensagem de erro no monitor serial.
        return; // Interrompe a execução da função caso a fila não tenha sido criada.
    }
}

// Loop principal
void loop() {
  // Monitora os perifericos
  IIKit.loop();
  void (*taskMessage)(); // Declaração de um ponteiro para uma função, que será recebido da fila. 
  // Verifica continuamente se há dados na fila
  while (xQueueReceive(taskQueue, &taskMessage, 0) == pdTRUE) {
        // Se xQueueReceive retorna pdTRUE, significa que há um item disponível na fila:
        // - O dado é removido da fila e armazenado em taskMessage.
        taskMessage();
    }  
}

//Funçao de alterar o estado de um led
void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

//Função que le os valores dos POT e das Entradas 4 a 20 mA e plota no display
void managerInputFunc(void) {
    const uint16_t vlPOT1 = IIKit.analogReadPot1();
    const uint16_t vlPOT2 = IIKit.analogReadPot2();
    const uint16_t vlR4a20_1 = IIKit.analogRead4a20_1();
    const uint16_t vlR4a20_2 = IIKit.analogRead4a20_2();

    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    IIKit.disp.setText(3, ("T1:" + String(vlR4a20_1) + "  T2:" + String(vlR4a20_2)).c_str());

    IIKit.WSerial.plot("vlPOT1", vlPOT1);
    IIKit.WSerial.plot("vlPOT2", vlPOT2);
    IIKit.WSerial.plot("vlR4a20_1", vlR4a20_1);
    IIKit.WSerial.plot("vlR4a20_2", vlR4a20_2);
}
