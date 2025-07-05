#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  pin_t pin_scl, pin_sda;
  pin_t ain[4];
  uint8_t pointer_reg;
  uint16_t config_reg;
  uint16_t conversion_reg;
  bool msb;
} chip_t;

static bool     on_i2c_connect(void *user_data, uint32_t i2c_index, uint32_t address);
static bool     on_i2c_write(void *user_data, uint32_t i2c_index, uint8_t data);
static uint8_t  on_i2c_read(void *user_data, uint32_t i2c_index);
static void     simulate_adc(void *user_data);

void chip_init() {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // AIN0–AIN3 configuradas como entradas analógicas
  for (int i = 0; i < 4; i++) {
    char name[6];
    snprintf(name, sizeof(name), "AIN%d", i);
    chip->ain[i] = pin_init(name, ANALOG);
  }

  // Configuração dos pinos I²C
  chip->pin_scl = pin_init("SCL", INPUT_PULLUP);
  chip->pin_sda = pin_init("SDA", INPUT_PULLUP);

  chip->pointer_reg = 0xFF;
  chip->msb = true;

  i2c_config_t cfg = {
    .address    = 0x48,
    .scl        = chip->pin_scl,
    .sda        = chip->pin_sda,
    .connect    = on_i2c_connect,
    .write      = on_i2c_write,
    .read       = on_i2c_read,
    .disconnect = NULL,
    .user_data  = chip
  };
  i2c_init(&cfg);

  timer_config_t tcfg = {
    .callback  = simulate_adc,
    .user_data = chip
  };
  timer_t t = timer_init(&tcfg);
  timer_start(t, 1000, true);

  printf("ADS1115 custom chip ready\n");
}

static bool on_i2c_connect(void *user_data, uint32_t i2c_index, uint32_t address) {
  (void)i2c_index;
  return address == 0x48;
}

static bool on_i2c_write(void *user_data, uint32_t i2c_index, uint8_t data) {
  chip_t *chip = user_data;
  (void)i2c_index;
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

static uint8_t on_i2c_read(void *user_data, uint32_t i2c_index) {
  chip_t *chip = user_data;
  (void)i2c_index;
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

static void simulate_adc(void *user_data) {
  chip_t *chip = user_data;
  uint8_t mux = (chip->config_reg >> 12) & 0x07;
  if (mux < 4) {
    float v = pin_adc_read(chip->ain[mux]);
    const float fsr = 4.096f;
    int16_t code = (int16_t)((v / fsr) * 32767);
    if (code < 0) code = 0;
    chip->conversion_reg = code;
  }
}
