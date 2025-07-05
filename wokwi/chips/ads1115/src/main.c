// Wokwi Custom Chip - ADS1115 Simulation
// SPDX-License-Identifier: MIT

#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

#define I2C_ADDRESS 0x48
#define REG_CONVERSION 0x00
#define REG_CONFIG     0x01

typedef struct {
  uint8_t current_reg;
  uint8_t config_buffer[2];
  uint8_t config_index;
  uint8_t read_index;
  uint16_t conversion_value;
  uint32_t ain_attr[4]; // AIN0 to AIN3
} chip_state_t;

static bool on_i2c_connect(void *user_data, uint32_t address, bool connect);
static bool on_i2c_write(void *user_data, uint8_t data);
static uint8_t on_i2c_read(void *user_data);

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->current_reg = 0;
  chip->config_index = 0;
  chip->read_index = 0;
  chip->conversion_value = 0;

  // Criando atributos AIN0 a AIN3 editáveis no Wokwi
  chip->ain_attr[0] = attr_init("AIN0", 1000);
  chip->ain_attr[1] = attr_init("AIN1", 2000);
  chip->ain_attr[2] = attr_init("AIN2", 3000);
  chip->ain_attr[3] = attr_init("AIN3", 4000);

  const i2c_config_t config = {
    .user_data = chip,
    .address = I2C_ADDRESS,
    .scl = pin_init("SCL", INPUT_PULLUP),
    .sda = pin_init("SDA", INPUT_PULLUP),
    .connect = on_i2c_connect,
    .write = on_i2c_write,
    .read = on_i2c_read
  };

  i2c_init(&config);

  printf("ADS1115 custom chip initialized!\n");
}

bool on_i2c_connect(void *user_data, uint32_t address, bool connect) {
  chip_state_t *chip = user_data;
  chip->config_index = 0;
  chip->read_index = 0;
  return true; // ACK
}

bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = user_data;

  if (chip->config_index == 0) {
    chip->current_reg = data;
  } else if (chip->config_index == 1 || chip->config_index == 2) {
    chip->config_buffer[chip->config_index - 1] = data;

    if (chip->current_reg == REG_CONFIG && chip->config_index == 2) {
      // Simular leitura do canal com base no MUX [14:12]
      uint8_t mux = (chip->config_buffer[0] >> 4) & 0x07;
      uint32_t value = 0;

      switch (mux) {
        case 0b100: value = attr_read(chip->ain_attr[0]); break; // AIN0
        case 0b101: value = attr_read(chip->ain_attr[1]); break; // AIN1
        case 0b110: value = attr_read(chip->ain_attr[2]); break; // AIN2
        case 0b111: value = attr_read(chip->ain_attr[3]); break; // AIN3
        default: value = 0; break; // Modo diferencial não implementado
      }

      chip->conversion_value = value & 0xFFFF;
    }
  }

  chip->config_index++;
  return true; // ACK
}

uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = user_data;
  uint8_t result = 0;

  if (chip->current_reg == REG_CONVERSION) {
    if (chip->read_index == 0) {
      result = (chip->conversion_value >> 8) & 0xFF;
    } else {
      result = chip->conversion_value & 0xFF;
    }
    chip->read_index++;
  }

  return result;
}