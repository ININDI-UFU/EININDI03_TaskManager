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

static bool     on_i2c_connect(void *user_data, uint32_t address, bool read);
static bool     on_i2c_write(void *user_data, uint8_t data);
static uint8_t  on_i2c_read(void *user_data);
static void     simulate_adc(void *user_data);

void chip_init() {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // Configura AIN0..AIN3 como entradas analógicas
  for (int i = 0; i < 4; i++) {
    char name[6];
    snprintf(name, sizeof(name), "AIN%d", i);
    chip->ain[i] = pin_init(name, ANALOG);
  }

  chip->pointer_reg = 0xFF;
  chip->msb = true;

  // Configuração I²C com callbacks corretos
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

  // Timer: executa simulate_adc() a cada ~1 ms
  timer_config_t tcfg = { .callback = simulate_adc, .user_data = chip };
  timer_t t = timer_init(&tcfg);
  timer_start(t, 1000, true);

  printf("ADS1115 custom chip ready\n");
}

static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  (void)read;
  return address == 0x48;
}

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