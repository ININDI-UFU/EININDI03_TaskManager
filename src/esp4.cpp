//https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
#include "IIKit.h"   // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#define MAXLENGTHJQUEUE 1024
#define NUMTASKS 3
#include "util/jtask.h"

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

// Configuração inicial do programa
void setup() {
    // Faz as configuções do hardware ESP + Perifericos
    IIKit.setup();
    jtaskSetup(1000);    // Configura o timer para 1000 Hz (1 ms)
    jtaskAttachFunc(managerInputFunc, 50); //anexa um função e sua base de tempo para ser executada
    jtaskAttachFunc([](){blinkLEDFunc(def_pin_D3);}, 500);  //anexa um função e sua base de tempo para ser executada
    //jtaskAttachFunc([](){blinkLEDFunc(def_pin_D4);}, 1000);  //anexa um função e sua base de tempo para ser executada
    IIKit.WSerial.onInputReceived([](String str) {
        if(str == "^q") IIKit.WSerial.stop(); 
        else IIKit.WSerial.print(str); 
        }
    );
    IIKit.rtn_1.onValueChanged([](uint8_t status) {digitalWrite(def_pin_D4,status);});
}

// Loop principal
void loop() {
  // Monitora os perifericos
  IIKit.loop();
  jtaskLoop();
}