#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  pin_t ain[4];
  uint8_t pointer_reg;
  uint16_t config_reg;
  uint8_t config_bytes[2]; // MSB, LSB
  uint16_t conversion_reg;
  bool msb;
  int config_byte_count;
} chip_t;

static bool     on_i2c_connect(void *user_data, uint32_t address, bool read);
static bool     on_i2c_write(void *user_data, uint8_t data);
static uint8_t  on_i2c_read(void *user_data);

// Helper para limitar faixa do ADC
static int16_t clamp(int32_t v) {
  if (v > 32767) return 32767;
  if (v < -32768) return -32768;
  return v;
}

void chip_init() {
  chip_t *chip = calloc(1, sizeof(chip_t));
  for (int i = 0; i < 4; i++) {
    char name[6];
    snprintf(name, sizeof(name), "AIN%d", i);
    chip->ain[i] = pin_init(name, ANALOG);
  }
  chip->pointer_reg = 0x00;
  chip->msb = true;
  chip->config_byte_count = 0;
  chip->config_bytes[0] = 0x85; // default power-on reset
  chip->config_bytes[1] = 0x83;

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

  printf("ADS1115 custom chip (full MUX, single/diff) pronto!\n");
}

static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  (void)read;
  return address == 0x48;
}

static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_t *chip = user_data;
  static bool expecting_pointer = true;

  if (expecting_pointer) {
    chip->pointer_reg = data;
    chip->msb = true;
    expecting_pointer = false;
    if (chip->pointer_reg == 0x01) chip->config_byte_count = 0;
  } else {
    if (chip->pointer_reg == 0x01) {
      chip->config_bytes[chip->config_byte_count++] = data;
      if (chip->config_byte_count == 2) {
        chip->config_reg = ((uint16_t)chip->config_bytes[0] << 8) | chip->config_bytes[1];
        chip->config_byte_count = 0;
      }
    }
    expecting_pointer = true;
  }
  return true;
}

static uint8_t on_i2c_read(void *user_data) {
  chip_t *chip = user_data;
  uint8_t out = 0xFF;

  if (chip->pointer_reg == 0x00) { // Conversion register
    if (chip->msb) {
      // --- MUX: 3 bits, bits 14:12 ---
      uint8_t mux = (chip->config_reg >> 12) & 0x07;
      float v = 0.0f;
      int16_t code = 0;

      // Referência do ADS1115: FSR=4.096V por padrão (PGA=±4.096V)
      const float fsr = 4.096f;
      float vin_p = 0, vin_n = 0, vdiff = 0;
      // Todos os modos do MUX (datasheet, Table 7)
      switch (mux) {
        case 0: // AIN0-AIN1 (diferencial)
          vin_p = pin_adc_read(chip->ain[0]);
          vin_n = pin_adc_read(chip->ain[1]);
          vdiff = vin_p - vin_n;
          code = clamp((int32_t)((vdiff / fsr) * 32767.0f));
          break;
        case 1: // AIN0-AIN3 (diferencial)
          vin_p = pin_adc_read(chip->ain[0]);
          vin_n = pin_adc_read(chip->ain[3]);
          vdiff = vin_p - vin_n;
          code = clamp((int32_t)((vdiff / fsr) * 32767.0f));
          break;
        case 2: // AIN1-AIN3 (diferencial)
          vin_p = pin_adc_read(chip->ain[1]);
          vin_n = pin_adc_read(chip->ain[3]);
          vdiff = vin_p - vin_n;
          code = clamp((int32_t)((vdiff / fsr) * 32767.0f));
          break;
        case 3: // AIN2-AIN3 (diferencial)
          vin_p = pin_adc_read(chip->ain[2]);
          vin_n = pin_adc_read(chip->ain[3]);
          vdiff = vin_p - vin_n;
          code = clamp((int32_t)((vdiff / fsr) * 32767.0f));
          break;
        case 4: // AIN0-GND (single-ended)
          v = pin_adc_read(chip->ain[0]);
          code = clamp((int32_t)((v / fsr) * 32767.0f));
          break;
        case 5: // AIN1-GND
          v = pin_adc_read(chip->ain[1]);
          code = clamp((int32_t)((v / fsr) * 32767.0f));
          break;
        case 6: // AIN2-GND
          v = pin_adc_read(chip->ain[2]);
          code = clamp((int32_t)((v / fsr) * 32767.0f));
          break;
        case 7: // AIN3-GND
          v = pin_adc_read(chip->ain[3]);
          code = clamp((int32_t)((v / fsr) * 32767.0f));
          break;
      }
      chip->conversion_reg = (uint16_t)code;

      // Debug log: mostra o tipo de conversão
      if (mux <= 3) {
        printf("[MUX=%d] Diferencial: Vin+ = %.3fV, Vin- = %.3fV, vdiff = %.3fV, code = %d\n",
          mux, vin_p, vin_n, vdiff, code);
      } else {
        printf("[MUX=%d] Single-ended: AIN%d = %.3fV, code = %d\n",
          mux, mux-4, v, code);
      }
    }
    out = chip->msb ? (chip->conversion_reg >> 8) : (chip->conversion_reg & 0xFF);
    chip->msb = !chip->msb;
  }
  else if (chip->pointer_reg == 0x01) { // Config register
    out = chip->msb ? (chip->config_reg >> 8) : (chip->config_reg & 0xFF);
    chip->msb = !chip->msb;
  }
  return out;
}