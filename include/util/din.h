/**
 * @file DIn_c.h
 * @brief Classe para gerenciamento de entradas digitais com suporte a debounce e callbacks no Arduino.
 *
 * Esta classe oferece recursos para lidar com entradas digitais utilizando debounce e ações baseadas em eventos, como mudança de estado ou pressão prolongada.
 */

#include <Arduino.h>
#include <functional>

#define DEBOUNCETIME 50 ///< Tempo de debounce em milissegundos.

/**
 * @class DIn_c
 * @brief Classe para manipulação de entradas digitais com debounce e eventos.
 */
class DIn_c {
protected:
    uint8_t pin; ///< Pino associado à entrada digital.
    volatile unsigned long reading_time = 0; ///< Tempo da última leitura.
    volatile uint8_t status_DIn = LOW; ///< Status atual da entrada digital.
    volatile uint8_t last_status_DIn = LOW; ///< Último status da entrada digital.
    volatile uint16_t pressed_DIn_time = 0; ///< Tempo configurado para pressão prolongada.
    std::function<void(uint8_t status)> onChanged = nullptr; ///< Callback para mudança de estado.
    std::function<void()> onPressed = nullptr; ///< Callback para pressão prolongada.

    /**
     * @brief Realiza o debounce na leitura da entrada digital.
     * @return true se o estado mudou após o debounce, false caso contrário.
     */
    bool debounce();

    /**
     * @brief Atualiza o estado da entrada digital e executa callbacks conforme necessário.
     */
    void update();

public:
    /**
     * @brief Construtor padrão.
     */
    DIn_c() { DIn_c(0); }

    /**
     * @brief Construtor que inicializa a classe com um pino especificado.
     * @param pinDIn Pino da entrada digital.
     */
    DIn_c(uint8_t pinDIn);

    /**
     * @brief Define um callback para mudança de estado da entrada.
     * @param f Função callback a ser executada.
     */
    void onValueChanged(std::function<void(uint8_t status)> f);

    /**
     * @brief Define um callback para pressão prolongada.
     * @param f Função callback a ser executada.
     */
    void onPressedWithTime(std::function<void()> f);

    /**
     * @brief Configura o pino da entrada digital.
     * @param pinDIn Pino da entrada digital.
     */
    void setPin(uint8_t pinDIn);

    /**
     * @brief Obtém o pino configurado para a entrada digital.
     * @return Pino configurado.
     */
    uint8_t getPin();

    /**
     * @brief Obtém o status atual da entrada digital.
     * @return Status da entrada digital (HIGH ou LOW).
     */
    uint8_t getStatus();

    /**
     * @brief Configura o tempo para o evento de pressão prolongada.
     * @param time Tempo em segundos.
     */
    void setTimeOnPressed(uint8_t time);

    /**
     * @brief Amigo da classe que permite atualizar a entrada digital externamente.
     * @param din Ponteiro para uma instância de DIn_c.
     */
    friend inline void updateDIn(DIn_c *din);          
};

/**
 * @brief Construtor que inicializa o pino da entrada digital.
 * @param pinDIn Pino da entrada digital.
 */
DIn_c::DIn_c(uint8_t pinDIn) {
    setPin(pinDIn);
}

/**
 * @brief Atualiza o estado da entrada digital.
 * @param din Ponteiro para uma instância de DIn_c.
 */
inline void updateDIn(DIn_c *din) {
    din->update();
}

void DIn_c::update() {
    if (debounce()) onChanged(status_DIn);
}

uint8_t DIn_c::getPin() {
    return pin;
}

void DIn_c::setPin(uint8_t pinDIn) {
    pin = pinDIn;
    pinMode(pin, INPUT_PULLDOWN);    
}

bool DIn_c::debounce() {
    uint8_t leitura = digitalRead(pin);
    if (pressed_DIn_time > 0 && leitura == HIGH && status_DIn == HIGH && (xTaskGetTickCount() - reading_time) > pressed_DIn_time) {
        reading_time = xTaskGetTickCount();
        onPressed();
    }
    if (leitura != last_status_DIn) {
        reading_time = xTaskGetTickCount();
        last_status_DIn = leitura;
    }
    if ((xTaskGetTickCount() - reading_time) > DEBOUNCETIME) {
        if (leitura != status_DIn) {
            status_DIn = leitura;
            return true;
        }
    }
    return false;
}

void DIn_c::setTimeOnPressed(uint8_t time) {
    pressed_DIn_time = ((uint16_t) time) * 1000;
}

uint8_t DIn_c::getStatus() {
    return status_DIn;
}

void DIn_c::onValueChanged(std::function<void(uint8_t status)> f) {
    onChanged = f;
}

void DIn_c::onPressedWithTime(std::function<void()> f) {
    onPressed = f;
}
