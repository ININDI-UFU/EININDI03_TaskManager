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

// Protótipos
static bool on_i2c_connect(void *user_data, uint32_t address, bool read);
static bool on_i2c_write(void *user_data, uint8_t data);
static uint8_t on_i2c_read(void *user_data);
static void simulate_adc(void *user_data);

// Função de inicialização (EXPORT)
void* EXPORT(chip_init)(void) {
  chip_t *chip = calloc(1, sizeof(chip_t));

  // Configurar pinos analógicos
  chip->ain[0] = pin_init("AIN0", ANALOG);
  chip->ain[1] = pin_init("AIN1", ANALOG);
  chip->ain[2] = pin_init("AIN2", ANALOG);
  chip->ain[3] = pin_init("AIN3", ANALOG);

  // Inicializar I2C com callbacks
  static const i2c_config_t cfg = {
    .address     = 0x48,
    .scl         = pin_init("SCL", INPUT_PULLUP),
    .sda         = pin_init("SDA", INPUT_PULLUP),
    .connect     = on_i2c_connect,
    .write       = on_i2c_write,
    .read        = on_i2c_read,
    .disconnect  = NULL,
    .user_data   = NULL // será ajustado abaixo
  };

  chip->i2c = i2c_init(&cfg);
  chip->i2c.user_data = chip;

  // Inicial pointer para indicar que próximo byte é pointer_reg
  chip->pointer_reg = 0xFF;

  // Timer para simular a conversão A/D a cada 1 ms
  timer_config_t tcfg = {
    .callback  = simulate_adc,
    .user_data = chip
  };
  timer_t t = timer_init(&tcfg);
  timer_start(t, 1000000, true); // 1 000 000 microsegundos = 1 ms

  return chip;
}

// Aceita conexão se o endereço estiver correto
static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_t *chip = user_data;
  (void)read;
  return address == chip->i2c.address;
}

// Escrita I2C: primeiro byte = pointer, depois payload
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

// Leitura I2C: retorna MSB/LSB conforme pointer_reg
static uint8_t on_i2c_read(void *user_data) {
  chip_t *chip = user_data;
  static bool high = true;
  uint8_t out = 0xFF;

  if (chip->pointer_reg == 0x00) {
    out = high ? (chip->conversion_reg >> 8) : (chip->conversion_reg & 0xFF);
    high = !high;
  } else if (chip->pointer_reg == 0x01) {
    out = high ? (chip->config_reg >> 8) : (chip->config_reg & 0xFF);
    high = !high;
  }

  return out;
}

// Timer: simula ADC lendo pino analógico e atualiza conversion_reg
static void simulate_adc(void *user_data) {
  chip_t *chip = user_data;
  uint8_t mux = (chip->config_reg >> 12) & 0x07;
  if (mux >= 4) return;

  float voltage = pin_adc_read(chip->ain[mux]);
  const float fsr = 4.096f;
  int16_t code = (int16_t)((voltage / fsr) * 32767.0f);
  if (code < 0) code = 0;
  chip->conversion_reg = (uint16_t)code;
}