/**
 * @file WSerial_c.h
 * @brief Classe para gerenciamento de conexões Telnet e comunicação serial.
 *
 * Esta classe estende a funcionalidade da biblioteca ESPTelnet, permitindo integração
 * com comunicação serial e suporte a recursos como gráficos e callbacks de eventos.
 */

#ifndef __WSERIAL_H
#define __WSERIAL_H

#include <Arduino.h>
#include "ESPTelnet.h"

#define BAUD_RATE 115200 ///< Taxa de transmissão padrão para comunicação serial.

/**
 * @class WSerial_c
 * @brief Classe para gerenciamento de conexões Telnet e comunicação serial.
 */
class WSerial_c : public ESPTelnet {
protected:
    /**
     * @brief Atualiza o estado da conexão Telnet e gerencia entradas seriais.
     */
    void update(void);

    /**
     * @brief Inicializa a conexão Telnet.
     *
     * @param port Porta para a conexão Telnet.
     * @param baudrate Taxa de transmissão para comunicação serial (padrão: BAUD_RATE).
     * @return true se a inicialização foi bem-sucedida, false caso contrário.
     */
    bool start(uint16_t port, unsigned long baudrate = BAUD_RATE);

public:
    /**
     * @brief Construtor da classe.
     */
    WSerial_c() : ESPTelnet() {}

    /**
     * @brief Envia dados para um gráfico utilizando x e y.
     *
     * @tparam T Tipo dos dados.
     * @param varName Nome da variável.
     * @param x Valor do eixo x.
     * @param y Valor do eixo y.
     * @param unit Unidade dos valores (opcional).
     */
    template <typename T>
    void plot(const char *varName, T x, T y, const char *unit = NULL);

    /**
     * @brief Envia dados para um gráfico utilizando apenas y.
     *
     * @tparam T Tipo dos dados.
     * @param varName Nome da variável.
     * @param y Valor do eixo y.
     * @param unit Unidade dos valores (opcional).
     */
    template <typename T>
    void plot(const char *varName, T y, const char *unit = NULL);

    /**
     * @brief Envia dados pela conexão Telnet ou Serial.
     *
     * @tparam T Tipo dos dados.
     * @param data Dados a serem enviados.
     */
    template <typename T>
    void print(const T &data);

    /**
     * @brief Envia dados com base específica pela conexão Telnet ou Serial.
     *
     * @tparam T Tipo dos dados.
     * @param data Dados a serem enviados.
     * @param base Base dos dados (e.g., binário, decimal).
     */
    template <typename T>
    void print(const T &data, int base);

    /**
     * @brief Envia dados com quebra de linha pela conexão Telnet ou Serial.
     *
     * @tparam T Tipo dos dados.
     * @param data Dados a serem enviados.
     */
    template <typename T>
    void println(const T &data);

    /**
     * @brief Envia dados com quebra de linha e base específica pela conexão Telnet ou Serial.
     *
     * @tparam T Tipo dos dados.
     * @param data Dados a serem enviados.
     * @param base Base dos dados (e.g., binário, decimal).
     */
    template <typename T>
    void println(const T &data, int base);

    /**
     * @brief Envia apenas uma quebra de linha.
     */
    void println();

    /**
     * @brief Obtém a porta do servidor Telnet.
     * @return Porta do servidor.
     */
    uint16_t serverPort() { return (this->server_port); }

    /**
     * @brief Verifica se a conexão Telnet está ativa.
     * @return true se conectado, false caso contrário.
     */
    bool isConnected();

    /**
     * @brief Função amiga para inicializar a conexão Telnet.
     *
     * @param ws Ponteiro para a instância de WSerial_c.
     * @param port Porta para a conexão Telnet.
     * @param baudrate Taxa de transmissão para comunicação serial.
     * @return true se a inicialização foi bem-sucedida, false caso contrário.
     */
    friend inline bool startWSerial(WSerial_c *ws, uint16_t port, unsigned long baudrate);

    /**
     * @brief Função amiga para atualizar a conexão Telnet.
     * @param ws Ponteiro para a instância de WSerial_c.
     */
    friend inline void updateWSerial(WSerial_c *ws);
};

inline bool WSerial_c::isConnected() {
    return ((ESPTelnet *) this)->isConnected();
}

inline bool startWSerial(WSerial_c *ws, uint16_t port, unsigned long baudrate) {
    return(ws->start(port, baudrate));
}

bool WSerial_c::start(uint16_t port, unsigned long baudrate) {
    if (isConnected()) {
        ((ESPTelnet *) this)->stop();
    }
    server_port = port;
    onDisconnect([](String ip) {
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.println(" disconnected");
    });
    onConnectionAttempt([](String ip) {
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.println(" tried to connected");
    });
    onReconnect([](String ip) {
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.println(" reconnected");
    });

    return (((ESPTelnet *) this)->begin(server_port));
}

inline void updateWSerial(WSerial_c *ws) {
    ws->update();
}

void WSerial_c::update(void) {
    if (isConnected()) {
        if (Serial.available() && on_input != NULL) {
            on_input(Serial.readStringUntil('\n'));
        }
    }
    ((ESPTelnet *) this)->loop();
}

template <typename T>
void WSerial_c::plot(const char *varName, T y, const char *unit) {
    plot(varName, (T)millis(), y, unit);
}

template <typename T>
void WSerial_c::plot(const char *varName, T x, T y, const char *unit) {
    print(">");
    print(varName);
    print(":");
    print(x);
    print(":");
    print(y);
    if (unit != NULL) {
        print("\u00a7");
        print(unit);
    }
    println("|g");
}

template <typename T>
void WSerial_c::print(const T &data) {
    if (((ESPTelnet *) this)->isConnected()) {
        ((ESPTelnet *) this)->print(data);
    } else {
        Serial.print(data);
    }
}

template <typename T>
void WSerial_c::print(const T &data, int base) {
    if (((ESPTelnet *) this)->isConnected()) {
        ((ESPTelnet *) this)->print(data, base);
    } else {
        Serial.print(data, base);
    }
}

template <typename T>
void WSerial_c::println(const T &data) {
    if (((ESPTelnet *) this)->isConnected()) {
        ((ESPTelnet *) this)->println(data);
    } else {
        Serial.println(data);
    }
}

template <typename T>
void WSerial_c::println(const T &data, int base) {
    if (((ESPTelnet *) this)->isConnected()) {
        ((ESPTelnet *) this)->println(data, base);
    } else {
        Serial.println(data, base);
    }
}

void WSerial_c::println() {
    if (((ESPTelnet *) this)->isConnected()) {
        ((ESPTelnet *) this)->println();
    } else {
        Serial.println();
    }
}

#endif