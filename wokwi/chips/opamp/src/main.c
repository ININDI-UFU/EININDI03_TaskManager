#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
  pin_t vinp, vinn, vout, vcc, vee;
  timer_t tid;
  int last_period;
  float last_gain;
} opamp_chip_t;

static void opamp_simulate(void *user_data);

void chip_init() {
  opamp_chip_t *chip = calloc(1, sizeof(opamp_chip_t));
  printf("[chip-opamp] Inicializando OpAmp custom\n");

  chip->vinp = pin_init("VINP", ANALOG);
  chip->vinn = pin_init("VINN", ANALOG);
  chip->vout = pin_init("VOUT", OUTPUT);
  chip->vcc  = pin_init("VCC", ANALOG);
  chip->vee  = pin_init("VEE", ANALOG);

  chip->last_period = 10;
  chip->last_gain = 100000.0f;

  // Configura timer inicial (pode ser atualizado dinamicamente)
  const timer_config_t tcfg = {
    .callback  = opamp_simulate,
    .user_data = chip,
  };
  chip->tid = timer_init(&tcfg);
  timer_start(chip->tid, chip->last_period, true);

  printf("[chip-opamp] OpAmp custom chip pronto com controles internos!\n");
}

static void opamp_simulate(void *user_data) {
  opamp_chip_t *chip = user_data;

  // Lê os valores dos controles internos
  float gain = control_get_value("GAIN");
  int period = (int)control_get_value("SampleTimeMs");
  if (period <= 0) period = 1;

  // Atualiza o timer se o período foi alterado
  if (period != chip->last_period) {
    timer_stop(chip->tid);
    timer_start(chip->tid, period, true);
    chip->last_period = period;
    printf("[chip-opamp] Novo período do timer: %d ms\n", period);
  }

  // Loga se o ganho mudar de valor significativamente
  if (fabs(gain - chip->last_gain) > 1.0f) {
    printf("[chip-opamp] Novo ganho: %.0f x\n", gain);
    chip->last_gain = gain;
  }

  // Leitura das entradas analógicas
  float vinp = pin_adc_read(chip->vinp);
  float vinn = pin_adc_read(chip->vinn);
  float vcc = chip->vcc ? pin_adc_read(chip->vcc) : 5.0f;
  float vee = chip->vee ? pin_adc_read(chip->vee) : 0.0f;

  // Cálculo do resultado do AmpOp (modelo ideal)
  float diff = vinp - vinn;
  float out = diff * gain;

  // Saturação em VCC e VEE
  if (out > vcc) out = vcc;
  if (out < vee) out = vee;

  pin_write(chip->vout, out);

  // Log de operação (ajuste conforme necessidade)
  printf("[chip-opamp] VINP=%.3fV VINN=%.3fV VOUT=%.3fV (Ganho=%.0fx, Periodo=%dms)\n",
         vinp, vinn, out, gain, period);
}