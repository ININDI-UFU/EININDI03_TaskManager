//https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
#include "IIKitmini.h"   // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#include "util/dinDebounce.h"

#ifndef NUMTASKS
  #define NUMTASKS 3
#endif

uint8_t jtaskIndex = 0;

struct TaskConfig_t {
  unsigned long lastExec;
  unsigned long period;
  void (*task)();
};


TaskConfig_t jtaskStruct[NUMTASKS];

bool jtaskAttachFunc(void (*task)(), unsigned long period) {
  if (jtaskIndex >= NUMTASKS) return false;  // Verifica se já atingiu o máximo de tarefas
  
  jtaskStruct[jtaskIndex].lastExec = micros();
  jtaskStruct[jtaskIndex].period   = period;
  jtaskStruct[jtaskIndex].task     = task;
  jtaskIndex++;
  return true;
}

void jtaskLoop() {
  unsigned long currentMicros = micros();
  for (uint8_t i = 0; i < jtaskIndex; i++) {
    if (currentMicros - jtaskStruct[i].lastExec >= jtaskStruct[i].period) {
      jtaskStruct[i].lastExec = currentMicros;
      jtaskStruct[i].task();
    }
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
  IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
  IIKit.WSerial.plot("vlPOT1", vlPOT1);
  IIKit.WSerial.plot("vlPOT2", vlPOT2);
}

DigitalINDebounce PUSH1(def_pin_PUSH1, 50, [](bool state){digitalWrite(def_pin_D3, state);});
DigitalINDebounce PUSH2(def_pin_PUSH2, 50, [](bool state){digitalWrite(def_pin_D4, state);});

// Configuração inicial do programa
void setup() {
  // Faz as configuções do hardware ESP + Perifericos
  IIKit.setup();
  jtaskAttachFunc(managerInputFunc, 50000UL); //anexa um função e sua base de tempo para ser executada
  jtaskAttachFunc([](){blinkLEDFunc(def_pin_D1);}, 500000UL);  //anexa um função e sua base de tempo para ser executada
  jtaskAttachFunc([](){blinkLEDFunc(def_pin_D2);}, 1000000UL);  //anexa um função e sua base de tempo para ser executada
}

// Loop principal
void loop() {
// Monitora os perifericos
IIKit.loop();
jtaskLoop();
PUSH1.update();  // Atualiza a leitura com debounce
PUSH2.update();  // Atualiza a leitura com debounce   
}