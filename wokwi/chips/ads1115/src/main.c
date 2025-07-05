#include "wokwi-api.h"
#include <stdlib.h>

// Estado do chip
typedef struct {
  i2c_dev_t   i2c;
  pin_t       ain[4];
  uint8_t     pointer_reg;
  uint16_t    config_reg;
  uint16_t    conversion_reg;
} chip_t;

// Protótipos das funções de callback
static bool     on_i2c_connect(void *user_data, uint32_t address, bool connect);
static bool     on_i2c_write(void *user_data, uint8_t data);
static uint8_t  on_i2c_read(void *user_data);
static void     simulate_adc(void *user_data);

// Inicialização
void EXPORT(chip_init)(void **state) {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // Endereço I2C padrão (0x48)
  chip->i2c.address = 0x48;
  // Registrar callbacks I2C
  i2c_register(chip_init, &chip->i2c, on_i2c_connect, on_i2c_write, on_i2c_read, chip);

  // Mapear pinos analógicos
  chip->ain[0] = pin_init("AIN0");
  chip->ain[1] = pin_init("AIN1");
  chip->ain[2] = pin_init("AIN2");
  chip->ain[3] = pin_init("AIN3");

  *state = chip;

  // Timer para simular ADC a cada 1 ms
  timer_init(EVENT_MS(1), simulate_adc, chip);
}

// Callback de conexão I2C: só aceita se for o endereço do chip
static bool on_i2c_connect(void *user_data, uint32_t address, bool connect) {
  chip_t *chip = (chip_t*)user_data;
  return address == chip->i2c.address;
}

// Callback de escrita I2C: lida com pointer register e registros
static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_t *chip = (chip_t*)user_data;

  if (chip->pointer_reg == 0xFF) {
    // Primeiro byte após start/restart define pointer
    chip->pointer_reg = data;
  } else {
    // Payload para o pointer atual
    switch (chip->pointer_reg) {
      case 0x01:  // registro de configuração (16 bits)
        chip->config_reg = (chip->config_reg << 8) | data;
        break;
      default:
        // Ignora outros registros
        break;
    }
    // Reset do pointer para o próximo comando
    chip->pointer_reg = 0xFF;
  }
  return true;
}

// Callback de leitura I2C: retorna byte de MSB/LSB do registrador selecionado
static uint8_t on_i2c_read(void *user_data) {
  chip_t *chip = (chip_t*)user_data;
  uint8_t out = 0;

  switch (chip->pointer_reg) {
    case 0x00: { // conversion register
      static bool high = true;
      if (high) {
        out = (chip->conversion_reg >> 8) & 0xFF;
      } else {
        out = chip->conversion_reg & 0xFF;
      }
      high = !high;
      break;
    }
    case 0x01: { // config register (se for lido)
      static bool high = true;
      if (high) {
        out = (chip->config_reg >> 8) & 0xFF;
      } else {
        out = chip->config_reg & 0xFF;
      }
      high = !high;
      break;
    }
    default:
      out = 0xFF;
  }

  return out;
}

// Função chamada pelo timer para atualizar conversion_reg
static void simulate_adc(void *user_data) {
  chip_t *chip = (chip_t*)user_data;

  // Seleciona canal pelos bits 12–14 de config_reg
  uint8_t mux = (chip->config_reg >> 12) & 0x07;
  if (mux >= 4) return;

  // Lê tensão do pino AINx (0.0 a VDD)
  float voltage = pin_adc_read(chip->ain[mux]);

  // FSR assumido: ±4.096 V (bits de PGA = 001)
  const float fsr = 4.096f;

  // Converte para código 16 bits (signed), mas garanta ≥0
  int16_t code = (int16_t)((voltage / fsr) * 32767.0f);
  if (code < 0) code = 0;

  chip->conversion_reg = (uint16_t)code;
}