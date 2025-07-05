#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  pin_t ain[4];
  uint8_t pointer_reg;
  uint16_t config_reg;
  uint16_t conversion_reg;
  bool msb;
} chip_t;

// Prototipos
static bool     on_i2c_connect(void *user_data, uint32_t address, bool read);
static bool     on_i2c_write(void *user_data, uint8_t data);
static uint8_t  on_i2c_read(void *user_data);

void chip_init() {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // Inicializa AIN0~AIN3
  for (int i = 0; i < 4; i++) {
    char name[6];
    snprintf(name, sizeof(name), "AIN%d", i);
    chip->ain[i] = pin_init(name, ANALOG);
  }

  chip->pointer_reg = 0x00; // Padrão: conversão
  chip->msb = true;

  i2c_config_t cfg = {
    .address    = 0x48,
    .scl        = pin_init("SCL", INPUT_PULLUP),
    .sda        = pin_init("SDA", INPUT_PULLUP),
    .connect    = on_i2c_connect,
    .write      = on_i2c_write,
    .read       = on_i2c_read,
    .disconnect = NULL,
    .user_data  = chip
  };
  i2c_init(&cfg);

  printf("ADS1115 custom chip (stable, muxed, on-demand) pronto!\n");
}

static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  (void)read;
  return address == 0x48;
}

static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_t *chip = user_data;

  // O primeiro byte após start define o pointer register
  static bool expecting_pointer = true;
  if (expecting_pointer) {
    chip->pointer_reg = data;
    chip->msb = true; // Sempre volta para MSB em nova leitura
    expecting_pointer = false;
  } else {
    if (chip->pointer_reg == 0x01) {
      // Registro de configuração recebe 2 bytes (MSB e LSB)
      chip->config_reg = (chip->config_reg << 8) | data;
    }
    expecting_pointer = true;
  }
  return true;
}

static uint8_t on_i2c_read(void *user_data) {
  chip_t *chip = user_data;
  uint8_t out = 0xFF;

  if (chip->pointer_reg == 0x00) { // Conversion register
    // Seleciona canal pelos bits do config_reg
    uint8_t mux = (chip->config_reg >> 12) & 0x07;
    if (mux < 4) {
      float v = pin_adc_read(chip->ain[mux]);
      const float fsr = 4.096f;
      int16_t code = (int16_t)((v / fsr) * 32767.0f);
      if (code < 0) code = 0;
      chip->conversion_reg = (uint16_t)code;
    } else {
      chip->conversion_reg = 0; // Se MUX inválido, retorna 0
    }

    out = chip->msb ? (chip->conversion_reg >> 8) : (chip->conversion_reg & 0xFF);
    chip->msb = !chip->msb; // Alterna entre MSB/LSB
  }
  else if (chip->pointer_reg == 0x01) { // Config register
    out = chip->msb ? (chip->config_reg >> 8) : (chip->config_reg & 0xFF);
    chip->msb = !chip->msb;
  }

  return out;
}