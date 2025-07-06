// SPDX-License-Identifier: MIT
// Wokwi Custom Chip - LM324 Quad OpAmp
#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  pin_t vcc, vee;
  struct {
    pin_t vinp, vinn, vout;
  } opamp[4];
  timer_t tid;
  uint32_t gain_attr, period_attr;
  float last_gain;
  int last_period;
} lm324_chip_t;

static void lm324_simulate(void *user_data);

void chip_init() {
  lm324_chip_t *chip = malloc(sizeof(lm324_chip_t));

  chip->vcc = pin_init("VCC", ANALOG);
  chip->vee = pin_init("VEE", ANALOG);

  // Inicializa cada um dos 4 amplificadores
  chip->opamp[0].vinp = pin_init("A1IN+", ANALOG);
  chip->opamp[0].vinn = pin_init("A1IN-", ANALOG);
  chip->opamp[0].vout = pin_init("A1OUT", OUTPUT);

  chip->opamp[1].vinp = pin_init("A2IN+", ANALOG);
  chip->opamp[1].vinn = pin_init("A2IN-", ANALOG);
  chip->opamp[1].vout = pin_init("A2OUT", OUTPUT);

  chip->opamp[2].vinp = pin_init("A3IN+", ANALOG);
  chip->opamp[2].vinn = pin_init("A3IN-", ANALOG);
  chip->opamp[2].vout = pin_init("A3OUT", OUTPUT);

  chip->opamp[3].vinp = pin_init("A4IN+", ANALOG);
  chip->opamp[3].vinn = pin_init("A4IN-", ANALOG);
  chip->opamp[3].vout = pin_init("A4OUT", OUTPUT);

  chip->gain_attr   = attr_init("gain",   100000);
  chip->period_attr = attr_init("period", 10);

  chip->last_gain   = 100000.0f;
  chip->last_period = 10;

  const timer_config_t tcfg = {
    .callback  = lm324_simulate,
    .user_data = chip,
  };
  chip->tid = timer_init(&tcfg);
  timer_start(chip->tid, chip->last_period, true);

  printf("[chip-lm324] LM324 Quad OpAmp custom pronto!\n");
}

static void lm324_simulate(void *user_data) {
  lm324_chip_t *chip = (lm324_chip_t*)user_data;

  float gain = (float)attr_read(chip->gain_attr);
  int period = (int)attr_read(chip->period_attr);
  if (period <= 0) period = 1;

  if (period != chip->last_period) {
    timer_stop(chip->tid);
    timer_start(chip->tid, period, true);
    chip->last_period = period;
    printf("[chip-lm324] Novo período do timer: %d ms\n", period);
  }
  if (fabs(gain - chip->last_gain) > 1.0f) {
    printf("[chip-lm324] Novo ganho: %.0fx\n", gain);
    chip->last_gain = gain;
  }

  float vcc = chip->vcc ? pin_adc_read(chip->vcc) : 5.0f;
  float vee = chip->vee ? pin_adc_read(chip->vee) : 0.0f;

  for (int i = 0; i < 4; i++) {
    float vinp = pin_adc_read(chip->opamp[i].vinp);
    float vinn = pin_adc_read(chip->opamp[i].vinn);
    float diff = vinp - vinn;
    float out = diff * gain;

    // Saturação em VCC/VEE
    if (out > vcc) out = vcc;
    if (out < vee) out = vee;

    pin_write(chip->opamp[i].vout, out);

    printf("[chip-lm324] OPAMP%d: VINP=%.3f VINN=%.3f VOUT=%.3f\n",
           i+1, vinp, vinn, out);
  }
}