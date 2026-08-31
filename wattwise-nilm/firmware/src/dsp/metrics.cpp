#include "dsp/metrics.h"
#include "config.h"
#include <Arduino.h>
#include <cmath>

static inline float removeBias(uint16_t raw) {
  return (float)((int)raw - ADC_MIDPOINT_COUNTS);
}

bool MetricsAccumulator::feed(const Sample& s, CycleMetrics& out) {
  const float v = removeBias(s.v);

  // v defasado ~90° (1/4 de ciclo atrás) para cálculo de potência reativa
  const uint32_t quarter = ADC_SAMPLES_PER_CYCLE / 4;
  const uint32_t idx90 = (hist_pos_ + ADC_SAMPLES_PER_CYCLE - quarter) % ADC_SAMPLES_PER_CYCLE;
  const float v90 = v_hist_[idx90];
  v_hist_[hist_pos_] = v;
  hist_pos_ = (hist_pos_ + 1) % ADC_SAMPLES_PER_CYCLE;

  sum_v2_ += (double)v * v;
  for (int c = 0; c < N_CURRENT_CHANNELS; ++c) {
    const float ic = removeBias(s.i[c]);
    sum_i2_[c] += (double)ic * ic;
    sum_vi_[c] += (double)v * ic;
    sum_vqi_[c] += (double)v90 * ic;
  }
  ++n_;

  // Cruzamento de zero ascendente da tensão fecha o ciclo (com histerese mínima).
  const bool zero_cross = (v_prev_ < 0.0f && v >= 0.0f);
  v_prev_ = v;
  if (!zero_cross || n_ < (uint32_t)(ADC_SAMPLES_PER_CYCLE / 2)) return false;

  const double inv = 1.0 / (double)n_;
  out.vrms = CAL_V_VOLTS_PER_COUNT * sqrtf((float)(sum_v2_ * inv));
  out.p_total = 0;
  for (int c = 0; c < N_CURRENT_CHANNELS; ++c) {
    out.irms[c] = CAL_I_AMPS_PER_COUNT * sqrtf((float)(sum_i2_[c] * inv));
    // P = média(v*i) convertida por calibração de V e I
    out.p[c] = CAL_V_VOLTS_PER_COUNT * CAL_I_AMPS_PER_COUNT * (float)(sum_vi_[c] * inv);
    out.q[c] = CAL_V_VOLTS_PER_COUNT * CAL_I_AMPS_PER_COUNT * (float)(sum_vqi_[c] * inv);
    const float s_app = out.vrms * out.irms[c];
    out.pf[c] = (s_app > 0.001f) ? (out.p[c] / s_app) : 0.0f;
    out.p_total += out.p[c];
  }
  out.t_ms = millis();

  // zera acumuladores para o próximo ciclo
  sum_v2_ = 0; n_ = 0;
  for (int c = 0; c < N_CURRENT_CHANNELS; ++c) { sum_i2_[c] = sum_vi_[c] = sum_vqi_[c] = 0; }
  return true;
}
