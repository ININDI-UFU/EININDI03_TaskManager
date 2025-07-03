// chips/ads1115.chip.c
// Wokwi Custom ADS1115 Chip with I²C handlers
// SPDX-License-Identifier: MIT

#include "../wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static pin_t ain[4];
static uint16_t configRegister = 0x8583;
static int selectedChannel = 0;

// Leitura interna do conversor A/D
static uint16_t readAdcValue() {
  printf("ADS1115: readAdcValue() – canal atual = %d\n", selectedChannel);
  float voltage = pin_adc_read(ain[selectedChannel]);
  printf("ADS1115: pin_adc_read retornou tensão = %f V\n", voltage);
  int16_t adc = (int16_t)(voltage * 32767);
  printf("ADS1115: convertido para valor ADC = %d\n", adc);
  return (uint16_t)adc;
}

// Handler de leitura I²C chamado pelo Wokwi quando o MCU faz Wire.requestFrom(...)
__attribute__((used, visibility("default")))
void chipI2CRead(uint8_t reg, uint8_t *data, uint8_t len) {
  printf("ADS1115: chipI2CRead() chamado – reg=0x%02X, len=%d\n", reg, len);

  if (reg == 0x00 && len == 2) {
    printf("ADS1115: lendo registrador de conversão (0x00)\n");
    uint16_t val = readAdcValue();
    data[0] = (val >> 8) & 0xFF;
    data[1] = val & 0xFF;
    printf("ADS1115: retornando MSB=0x%02X, LSB=0x%02X\n", data[0], data[1]);
  } else {
    printf("ADS1115: registrador desconhecido ou tamanho inválido, preenchendo zeros\n");
    for (int i = 0; i < len; i++) {
      data[i] = 0;
    }
  }

  printf("ADS1115: chipI2CRead() concluído\n");
}

// Handler de escrita I²C chamado pelo Wokwi quando o MCU faz Wire.write(...)
__attribute__((used, visibility("default")))
void chipI2CWrite(uint8_t reg, const uint8_t *data, uint8_t len) {
  printf("ADS1115: chipI2CWrite() chamado – reg=0x%02X, len=%d\n", reg, len);

  if (reg == 0x01 && len == 2) {
    uint16_t oldCfg = configRegister;
    configRegister = ((uint16_t)data[0] << 8) | data[1];
    printf("ADS1115: configRegister alterado de 0x%04X para 0x%04X\n", oldCfg, configRegister);

    int mux = (configRegister >> 12) & 0x07;
    printf("ADS1115: novo MUX = 0b%03d\n", mux);
    switch (mux) {
      case 0b100: selectedChannel = 0; break;
      case 0b101: selectedChannel = 1; break;
      case 0b110: selectedChannel = 2; break;
      case 0b111: selectedChannel = 3; break;
      default:
        selectedChannel = 0;
        printf("ADS1115: MUX inválido, revertendo canal 0\n");
        break;
    }
    printf("ADS1115: selectedChannel = %d\n", selectedChannel);
  } else {
    printf("ADS1115: gravação em registrador não suportado ou len inválido\n");
  }

  printf("ADS1115: chipI2CWrite() concluído\n");
}

// Entrada chamada pelo Wokwi ao instanciar o chip
__attribute__((used, visibility("default")))
void chipInit(void) {
  printf("ADS1115: chipInit() iniciado\n");

  ain[0] = pin_init("AIN0", ANALOG);
  printf("ADS1115: pino AIN0 inicializado (id=%d)\n", ain[0]);

  ain[1] = pin_init("AIN1", ANALOG);
  printf("ADS1115: pino AIN1 inicializado (id=%d)\n", ain[1]);

  ain[2] = pin_init("AIN2", ANALOG);
  printf("ADS1115: pino AIN2 inicializado (id=%d)\n", ain[2]);

  ain[3] = pin_init("AIN3", ANALOG);
  printf("ADS1115: pino AIN3 inicializado (id=%d)\n", ain[3]);

  printf("ADS1115: chipInit() concluído, canal inicial = %d\n", selectedChannel);
}