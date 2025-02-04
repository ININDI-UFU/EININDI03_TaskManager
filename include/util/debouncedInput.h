#ifndef DEBOUNCEDINPUT_H
#define DEBOUNCEDINPUT_H

#include <Arduino.h>

/**
 * @brief Classe para gerenciamento de entradas digitais com interrupção e debounce.
 *
 * Esta classe encapsula a funcionalidade de leitura de uma entrada digital utilizando
 * interrupções e tratamento de debounce. Permite configurar o pino, o modo de disparo
 * da interrupção, o tempo de debounce e o comportamento (toggle ou leitura direta).
 */
class DebouncedInput {
public:
    /**
     * @brief Tipo da função callback.
     *
     * A função callback deve ser do tipo que recebe um parâmetro booleano representando o novo estado.
     */
    typedef void (*CallbackType)(bool state);

    /**
     * @brief Inicializa a entrada digital e configura a interrupção.
     *
     * Configura o pino como INPUT_PULLUP e registra a interrupção utilizando attachInterruptArg.
     *
     *
     * @param pin Número do pino de entrada digital.
     * @param mode Modo da interrupção (ex: FALLING, RISING, CHANGE).
     * @param debounceDelay Tempo de debounce em microssegundos (padrão 50000µs = 50ms).
     * @param toggle Se true, o estado interno é alternado a cada interrupção válida;
     *               se false, o estado reflete a leitura direta do pino.
     */
    void setup(uint8_t pin, int mode = FALLING, unsigned long debounceDelay = 50000UL, bool toggle = false)
    {
        _pin = pin;
        _mode = mode;
        _debounceDelay = debounceDelay;
        _toggle = toggle;
        _lastInterruptTime = 0;
        _state = false;
        _changed = false;
        _callback = nullptr;
        pinMode(_pin, INPUT_PULLUP);
        attachInterruptArg(digitalPinToInterrupt(_pin), isr, this, _mode);        
    }
    /**
     * @brief Define a função callback que será chamada no loop quando houver alteração.
     *
     * @param cb Ponteiro para a função callback que recebe o novo estado da entrada.
     */
    void setCallback(CallbackType cb) {
        _callback = cb;
    }

    /**
     * @brief Processa a alteração de estado no loop principal.
     *
     * Caso tenha ocorrido uma alteração válida (_changed == true) e a callback tenha sido definida,
     * esta função é chamada com o novo estado. Após o processamento, a flag _changed é resetada.
     */
    void update() {
        if (_changed) {
            if (_callback) {
                _callback(_state);
            }
            _changed = false;
        }
    }

    /**
     * @brief Retorna o estado atual (debounced) da entrada.
     *
     * @return Estado atual da entrada (true ou false).
     */
    bool getState() {
        return _state;
    }

private:
    /**
     * @brief Função de interrupção estática que redireciona para a instância correta.
     *
     * @param arg Ponteiro para a instância da classe DebouncedInput.
     */
    static void IRAM_ATTR isr(void* arg) __attribute__((noinline)) {
        DebouncedInput* instance = static_cast<DebouncedInput*>(arg);
        instance->handleInterrupt();
    }

    /**
     * @brief Tratamento da interrupção com debounce.
     *
     * Este método é executado no contexto da interrupção. Verifica se o tempo decorrido desde a
     * última interrupção é maior que o debounceDelay. Se sim, atualiza o estado da entrada (modo toggle
     * ou leitura direta) e define a flag _changed para processamento posterior no loop.
     */
    void IRAM_ATTR handleInterrupt() {
        unsigned long currentTime = micros();
        if (currentTime - _lastInterruptTime > _debounceDelay) {
            _lastInterruptTime = currentTime;
            if (_toggle) {
                _state = !_state;
            } else {
                // Utiliza gpio_get_level() que é segura para execução em IRAM
                _state = (gpio_get_level((gpio_num_t)_pin) != 0);
            }
            _changed = true;
        }
    }

    uint8_t _pin;                 ///< Pino de entrada digital.
    int _mode;                    ///< Modo de interrupção (FALLING, RISING, CHANGE).
    unsigned long _debounceDelay; ///< Tempo de debounce em microssegundos.
    bool _toggle;                 ///< Se true, ativa o modo toggle: alterna o estado a cada interrupção.
    volatile unsigned long _lastInterruptTime; ///< Armazena o tempo da última interrupção válida.
    volatile bool _state;         ///< Estado atual (debounced) da entrada.
    volatile bool _changed;       ///< Flag indicando que houve alteração no estado.
    CallbackType _callback;       ///< Ponteiro para a função callback a ser executada.
};

#endif // DEBOUNCEDINPUT_H