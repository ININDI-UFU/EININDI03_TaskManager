#include "wokwi-api.h"
#include <stdlib.h>

#define I2C_ADDRESS 0x48

// Registradores
#define REG_CONVERSION 0x00
#define REG_CONFIG     0x01

typedef struct {
  i2c_dev_t i2c_dev;
  i2c_config_t i2c_config;
  uint8_t current_reg;
  uint8_t config_buffer[2];
  uint8_t config_index;
  uint16_t conversion_value;
  uint32_t analog_inputs[4];// AIN0 - AIN3
} chip_state_t;

static chip_state_t chip;

void chip_init();
bool on_i2c_connect(void *user_data, uint32_t address, bool read);
bool on_i2c_write(void *user_data, uint8_t data);
uint8_t on_i2c_read(void *user_data);

void chip_init() {
  chip.i2c_config.address = I2C_ADDRESS;
  chip.i2c_config.scl = pin_init("SCL", INPUT_PULLUP);
  chip.i2c_config.sda = pin_init("SDA", INPUT_PULLUP);
  chip.i2c_config.user_data = &chip;
  chip.i2c_config.connect = on_i2c_connect;
  chip.i2c_config.write = on_i2c_write;
  chip.i2c_config.read = on_i2c_read;

  chip.current_reg = 0;
  chip.config_index = 0;
  chip.conversion_value = 0;

  chip.analog_inputs[0] = attr_init("AIN0", 1000);
  chip.analog_inputs[1] = attr_init("AIN1", 2000);
  chip.analog_inputs[2] = attr_init("AIN2", 3000);
  chip.analog_inputs[3] = attr_init("AIN3", 4000);

  i2c_init(&chip.i2c_config);
}

// Simula conversão de um canal baseado na configuração
static void update_conversion() {
  // MUX bits [14:12] da config_buffer determinam o canal
  uint8_t mux = (chip.config_buffer[0] >> 4) & 0x07;

  uint16_t value = 0;
  switch (mux) {
    case 0b100: value = attr_read(chip.analog_inputs[0]); break; // AIN0
    case 0b101: value = attr_read(chip.analog_inputs[1]); break; // AIN1
    case 0b110: value = attr_read(chip.analog_inputs[2]); break; // AIN2
    case 0b111: value = attr_read(chip.analog_inputs[3]); break; // AIN3
    default: value = 0; break; // Não suportado: entradas diferenciais
  }

  chip.conversion_value = value & 0xFFFF;
}

bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip.config_index = 0;
  return true; // ACK
}

bool on_i2c_write(void *user_data, uint8_t data) {
  if (chip.config_index == 0) {
    chip.current_reg = data;
  } else if (chip.config_index == 1 || chip.config_index == 2) {
    chip.config_buffer[chip.config_index - 1] = data;
    if (chip.current_reg == REG_CONFIG && chip.config_index == 2) {
      update_conversion();
    }
  }
  chip.config_index++;
  return true;
}

uint8_t on_i2c_read(void *user_data) {
  uint8_t result = 0;
  static uint8_t read_step = 0;

  if (chip.current_reg == REG_CONVERSION) {
    if (read_step == 0) {
      result = (chip.conversion_value >> 8) & 0xFF;
    } else {
      result = chip.conversion_value & 0xFF;
    }
    read_step = (read_step + 1) % 2;
  }

  return result;
}