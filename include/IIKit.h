/**
 * @file InIndKit.h
 * @brief Classe para gerenciamento de dispositivos industriais utilizando ESP32.
 *
 * Esta classe oferece suporte para gerenciamento de entradas digitais, saídas analógicas,
 * comunicação via Wi-Fi, OTA (Over-The-Air), display OLED e outras funcionalidades
 * industriais baseadas em ESP32.
 */

#ifndef __ININDKIT_H
#define __ININDKIT_H

#include <HardwareSerial.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <WiFi.h>

#include "services/OTA.h"
#include "services/WSerial_c.h"
#include "services/display_c.h"
#include "services/wifimanager_c.h"

#include "util/asyncDelay.h"
#include "util/din.h"
#include "services/ads1115_c.h"
#include "services/hart_c.h"

/********** GPIO DEFINITIONS ***********/
#define def_pin_RELE 39    ///< GPIO para relé.
#define def_pin_PWM 36     ///< GPIO para saída PWM.
#define def_pin_D1 35      ///< GPIO para I/O digital 1.
#define def_pin_D2 34      ///< GPIO para I/O digital 2.
#define def_pin_D4 33      ///< GPIO para I/O digital 4.
#define def_pin_ADC1 32    ///< GPIO para entrada ADC1.
#define def_pin_D3 27      ///< GPIO para I/O digital 3.
#define def_pin_W4a20_1 26 ///< GPIO para saída 4-20mA 1.
#define def_pin_DAC1 25    ///< GPIO para saída DAC1.
#define def_pin_PUSH1 23   ///< GPIO para botão push 1.
#define def_pin_SCL 22     ///< GPIO para SCL do display OLED.
#define def_pin_SDA 21     ///< GPIO para SDA do display OLED.
#define def_pin_RTN2 19    ///< GPIO para botão retentivo 2.
#define def_pin_RTN1 18    ///< GPIO para botão retentivo 1.
///< GPIO15 - ESP_PROG_TDO:6
///< GPIO14 - ESP_PROG_TMS:2
///< GPIO13 - ESP_PROG_TCK:4
///< GPIO12 - ESP_PROG_TDI:8
#define def_pin_PUSH2 4    ///< GPIO para botão push 2.
///< GPIO3  - ESP_COM_TX:3
///< GPIO1  - ESP_COM_RX:5
///< GPIO0  - ESP_COM_BOOT:6
///< ESPEN  - ESP_COM_EN:1

/**
 * @class IIKit_c
 * @brief Classe para gerenciamento do kit industrial.
 */
class IIKit_c
{
private:
    char DDNSName[15] = "inindkit"; ///< Nome do dispositivo para DDNS.
    WifiManager_c wm;               ///< Gerenciador de conexões Wi-Fi.
    ADS1115_c ads;                  ///< Conversor ADC.
    Hart_c hart;

    /**
     * @brief Exibe mensagens de erro e reinicia o dispositivo se necessário.
     * @param error Mensagem de erro.
     * @param restart Indica se o dispositivo deve ser reiniciado.
     */
    void errorMsg(String error, bool restart = true);

public:
    DIn_c rtn_1;       ///< Botão de retorno 1.
    DIn_c rtn_2;       ///< Botão de retorno 2.
    DIn_c push_1;      ///< Botão push 1.
    DIn_c push_2;      ///< Botão push 2.
    Display_c disp;    ///< Display OLED.
    WSerial_c WSerial; ///< Conexão Telnet e Serial.

    /**
     * @brief Inicializa o kit industrial.
     */
    void setup();

    /**
     * @brief Executa o loop principal do kit industrial.
     */
    void loop(void);

    /**
     * @brief Lê o valor do potenciômetro 1.
     * @return Valor analógico do potenciômetro 1.
     */
    uint16_t analogReadPot1(void);

    /**
     * @brief Lê o valor do potenciômetro 2.
     * @return Valor analógico do potenciômetro 2.
     */
    uint16_t analogReadPot2(void);

    /**
     * @brief Lê o valor do canal 4-20mA 1.
     * @return Valor analógico do canal 4-20mA 1.
     */
    uint16_t analogRead4a20_1(void);

    /**
     * @brief Lê o valor do canal 4-20mA 2.
     * @return Valor analógico do canal 4-20mA 2.
     */
    uint16_t analogRead4a20_2(void);
};

inline void IIKit_c::setup()
{
    WSerial.println("Booting");
    hart.setup(&WSerial);
    /********** Inicializando Display ***********/
    if (startDisplay(&disp, def_pin_SDA, def_pin_SCL))
    {
        disp.setText(1, "Inicializando...");
        WSerial.println("Display running");
    }
    else
    {
        errorMsg("Display error.", false);
    }

    delay(50);

    /********** Inicializando EEPROM ***********/
    EEPROM.begin(1);
    char idKit[2] = "0";
    //EEPROM.write(0, (uint8_t)idKit[0]);
    //EEPROM.commit();
    idKit[0] = (char)EEPROM.read(0);
    strcat(DDNSName, idKit);

    /********** Configurando Wi-Fi ***********/
    WiFi.mode(WIFI_STA);
    wm.start(&WSerial);
    wm.setApName(DDNSName);

    disp.setFuncMode(true);
    disp.setText(1, "Mode: Acces Point", true);
    disp.setText(2, "SSID: AutoConnectAP", true);
    disp.setText(3, "PSWD: ", true);

    if (wm.autoConnect("AutoConnectAP"))
    {
        WSerial.print("\nWifi running - IP:");
        WSerial.println(WiFi.localIP());
        disp.setFuncMode(false);
        disp.setText(1, (WiFi.localIP().toString() + " ID:" + String(idKit[0])).c_str());
        disp.setText(2, DDNSName);
        disp.setText(3, "UFU Mode");
        delay(50);
    }
    else
    {
        errorMsg("Wifi error.\nAP MODE...", false);
    }

    /********** Inicializando OTA ***********/
    OTA::start(DDNSName);

    /********** Inicializando Telnet ***********/
    startWSerial(&WSerial, 4000 + String(idKit[0]).toInt(), 115200);

    /********** Configurando GPIOs ***********/
    pinMode(def_pin_RTN1, INPUT_PULLDOWN);
    pinMode(def_pin_RTN2, INPUT_PULLDOWN);
    pinMode(def_pin_PUSH1, INPUT_PULLDOWN);
    pinMode(def_pin_PUSH2, INPUT_PULLDOWN);
    pinMode(def_pin_D1, OUTPUT);
    pinMode(def_pin_D2, OUTPUT);
    pinMode(def_pin_D3, OUTPUT);
    pinMode(def_pin_D4, OUTPUT);
    pinMode(def_pin_PWM, OUTPUT);
    pinMode(def_pin_DAC1, ANALOG);
    pinMode(def_pin_ADC1, ANALOG);
    pinMode(def_pin_RELE, OUTPUT);
    pinMode(def_pin_W4a20_1, OUTPUT);

    rtn_1.setPin(def_pin_RTN1);
    rtn_2.setPin(def_pin_RTN2);
    push_1.setPin(def_pin_PUSH1);
    push_2.setPin(def_pin_PUSH2);

    push_1.setTimeOnPressed(3);
    push_1.onPressedWithTime([this]()
                             {
        if (wm.changeWebPortal()) {
            disp.setFuncMode(true);
            disp.setText(2, "Web Portal ON", true);
            disp.setText(3, "", true);
        } else {
            disp.setFuncMode(false);
            disp.setText(2, DDNSName);
        } });
    digitalWrite(def_pin_D1, LOW);
    digitalWrite(def_pin_D2, LOW);        
    digitalWrite(def_pin_D3, LOW);
    digitalWrite(def_pin_D4, LOW);
    digitalWrite(def_pin_RELE, LOW);
    analogWrite(def_pin_PWM, 0);
    analogWrite(def_pin_DAC1, 0);
    analogWrite(def_pin_W4a20_1, 0);

    if (!ads.begin())
    {
        errorMsg("ADS error.", true);
    }
}

void IIKit_c::loop(void)
{
    OTA::handle();
    updateWSerial(&WSerial);
    updateDisplay(&disp);

    if (wm.getPortalRunning())
    {
        wm.process();
    }

    updateDIn(&rtn_1);
    updateDIn(&rtn_2);
    updateDIn(&push_1);
    updateDIn(&push_2);
}

uint16_t IIKit_c::analogReadPot1(void)
{
    return ads.analogRead(1);
}

uint16_t IIKit_c::analogReadPot2(void)
{
    return ads.analogRead(0);
}

uint16_t IIKit_c::analogRead4a20_1(void)
{
    return ads.analogRead(3);
}

uint16_t IIKit_c::analogRead4a20_2(void)
{
    return ads.analogRead(2);
}

void IIKit_c::errorMsg(String error, bool restart)
{
    WSerial.println(error);
    if (restart)
    {
        WSerial.println("Rebooting now...");
        delay(2000);
        ESP.restart();
    }
}

IIKit_c IIKit;

#endif