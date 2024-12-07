//https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
#include "IIKit.h"   // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#include "util/task.h"

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

CounterConfig taskStruct[] = {
    {0, 500, [](){blinkLEDFunc(def_pin_D1);}},
    {0, 50, managerInputFunc}
};

// Configuração inicial do programa
void setup() {
    // Faz as configuções do hardware ESP + Perifericos
    IIKit.setup();
    pinMode(def_pin_D1, OUTPUT);
    // Configuração do timer
    taskSetup(1000,1024);
}

// Loop principal
void loop() {
  // Monitora os perifericos
  IIKit.loop();
  taskLoop();
}