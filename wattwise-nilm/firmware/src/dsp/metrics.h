#pragma once
#include <cstdint>
#include "afe/sampler.h"

// Grandezas elétricas calculadas sobre um ciclo completo da rede.
struct CycleMetrics {
  float irms[3];   // A, por canal
  float vrms;      // V
  float p[3];      // potência ativa (W) por canal
  float q[3];      // potência reativa (VAr) por canal (via defasagem de 90°)
  float pf[3];     // fator de potência por canal
  float p_total;   // soma das potências ativas
  uint32_t t_ms;   // timestamp (millis) do fim do ciclo
};

// Acumulador que integra amostras até fechar um ciclo (detectado por cruzamento de zero
// da tensão) e então emite CycleMetrics. Remove o offset DC (bias Vcc/2) e aplica os
// fatores de calibração de config.h.
class MetricsAccumulator {
 public:
  // Alimenta uma amostra. Retorna true e preenche 'out' quando um ciclo fecha.
  bool feed(const Sample& s, CycleMetrics& out);

 private:
  // acumuladores de soma de quadrados e produtos v*i por ciclo
  double sum_i2_[3] = {0, 0, 0};
  double sum_v2_ = 0;
  double sum_vi_[3] = {0, 0, 0};       // para potência ativa
  double sum_vqi_[3] = {0, 0, 0};      // v defasado 90° * i, para reativa
  uint32_t n_ = 0;
  float v_prev_ = 0;                    // p/ detectar cruzamento de zero ascendente
  // buffer curto de v para o deslocamento de 90° (1/4 de ciclo)
  float v_hist_[ADC_SAMPLES_PER_CYCLE] = {0};
  uint32_t hist_pos_ = 0;
};
