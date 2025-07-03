// Wokwi Custom Op-Amp Chip
// SPDX-License-Identifier: MIT

#include "..\wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t  in_plus;
  pin_t  in_minus;
  pin_t  out;
  int    gain_attr;
} opamp_data_t;

// Callback executado periodicamente para processar o sinal
void chip_timer_callback(void *udata) {
  opamp_data_t *d = (opamp_data_t*)udata;

  // Leitura das tensões de entrada
  float vp = pin_adc_read(d->in_plus);
  float vm = pin_adc_read(d->in_minus);
  printf("OpAmp: IN+ = %f V, IN- = %f V\n", vp, vm);

  // Leitura do ganho configurado pelo usuário
  int gain = attr_read(d->gain_attr);
  printf("OpAmp: ganho configurado = %d\n", gain);

  // Cálculo da saída: Vout = (IN+ – IN-) * ganho
  float vout = (vp - vm) * gain;
  // Limita entre 0 e 3.3 V (faixa típica do DAC)
  if (vout < 0.0f)   vout = 0.0f;
  if (vout > 3.3f)   vout = 3.3f;
  printf("OpAmp: Vout calculado = %f V\n", vout);

  // Escreve no pino de saída
  pin_dac_write(d->out, vout);
}

// Inicialização do chip
__attribute__((used, visibility("default")))
void chipInit(void) {
  // Aloca estrutura de dados
  opamp_data_t *d = malloc(sizeof(opamp_data_t));
  if (!d) {
    printf("OpAmp: erro ao alocar memória\n");
    return;
  }

  // Configura pinos de I/O
  d->in_plus  = pin_init("IN+",  ANALOG);
  d->in_minus = pin_init("IN-",  ANALOG);
  d->out      = pin_init("OUT",  ANALOG);

  // Cria atributo 'gain' (fator de multiplicação)
  // Usuário poderá ajustar este valor pela UI do Wokwi
  d->gain_attr = attr_init("gain", 1);

  printf("OpAmp: inicializado com IN+ id=%d, IN- id=%d, OUT id=%d, ganho id=%d\n",
         d->in_plus, d->in_minus, d->out, d->gain_attr);

  // Configura timer para chamar a cada 100 ms (10 Hz)
  const timer_config_t tcfg = {
    .callback  = chip_timer_callback,
    .user_data = d,
  };
  timer_t tid = timer_init(&tcfg);
  timer_start(tid, 100, true);
}