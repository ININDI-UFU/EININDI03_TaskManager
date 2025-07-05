#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

// Estado do chip
typedef struct {
  i2c_dev_t   i2c;
  pin_t       ain[4];
  uint8_t     pointer_reg;
  uint16_t    config_reg;
  uint16_t    conversion_reg;
  bool        msb;
} chip_t;

// Protótipos
static bool     on_i2c_connect(void *user_data, uint32_t address, bool read);
static bool     on_i2c_write(void *user_data, uint8_t data);
static uint8_t  on_i2c_read(void *user_data);
static void     simulate_adc(void *user_data);

// Inicialização do chip
void EXPORT(chip_init)(void) {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // Inicializa pinos analógicos AIN0–AIN3
  for (int i = 0; i < 4; i++) {
    char name[6];
    snprintf(name, sizeof(name), "AIN%d", i);
    chip->ain[i] = pin_init(name, ANALOG);
  }

  // Configuração I²C
  const i2c_config_t cfg = {
    .address     = 0x48,
    .scl         = pin_init("SCL", INPUT_PULLUP),
    .sda         = pin_init("SDA", INPUT_PULLUP),
    .connect     = on_i2c_connect,
    .write       = on_i2c_write,
    .read        = on_i2c_read,
    .disconnect  = NULL,
    .user_data   = chip
  };
  chip->i2c = i2c_init(&cfg);

  chip->pointer_reg = 0xFF;
  chip->msb = true;

  // Timer para simulação ADC (~1 ms)
  const timer_config_t tcfg = {
    .callback  = simulate_adc,
    .user_data = chip
  };
  timer_t t = timer_init(&tcfg);
  timer_start(t, 1000, true);

  printf("ADS1115 initialized\n");
}

// Valida se o endereço I²C corresponde
static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_t *chip = user_data;
  return address == chip->i2c.address;
}

// Escrita I²C (pointer ou config)
static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_t *chip = user_data;
  if (chip->pointer_reg == 0xFF) {
    chip->pointer_reg = data;
  } else {
    if (chip->pointer_reg == 0x01) {
      chip->config_reg = (chip->config_reg << 8) | data;
    }
    chip->pointer_reg = 0xFF;
  }
  return true;
}

// Leitura I²C (MSB/LSB de conversão ou config)
static uint8_t on_i2c_read(void *user_data) {
  chip_t *chip = user_data;
  uint8_t out = 0xFF;
  if (chip->pointer_reg == 0x00) {
    out = chip->msb ? (chip->conversion_reg >> 8) : (chip->conversion_reg & 0xFF);
    chip->msb = !chip->msb;
  } else if (chip->pointer_reg == 0x01) {
    out = chip->msb ? (chip->config_reg >> 8) : (chip->config_reg & 0xFF);
    chip->msb = !chip->msb;
  }
  return out;
}

// Atualiza resposta ADC a cada ~1 ms
static void simulate_adc(void *user_data) {
  chip_t *chip = user_data;
  uint8_t mux = (chip->config_reg >> 12) & 0x07;
  if (mux < 4) {
    float v = pin_adc_read(chip->ain[mux]);
    const float fsr = 4.096f;
    int16_t code = (int16_t)((v / fsr) * 32767.0f);
    if (code < 0) code = 0;
    chip->conversion_reg = (uint16_t)code;
  }
}