#pragma once
#include <cstdint>
#include "nilm/event_detector.h"

// Vetor de features de um evento — MESMA definição usada no treino Python
// (ml/extract_features.py). A ordem e o significado DEVEM bater exatamente.
#define N_FEATURES 20

struct FeatureVector {
  float f[N_FEATURES];
  // Layout:
  //  0: delta_p (W)          1: delta_q (VAr)     2: delta_s (VA)
  //  3: pf_before            4: pf_after          5: sign (+1/-1)
  //  6: inrush_ratio         7: t_settle_ms
  //  8..14: H1..H7 (magnitude relativa das harmônicas ímpares da corrente)
  // 15..19: reservado / futuras features
};

// Monta o vetor a partir do evento + (opcional) harmônicas já calculadas por FFT
// sobre a janela do transiente. 'harmonics' aponta para N_HARMONICS floats normalizados.
void buildFeatureVector(const SwitchEvent& ev, const float* harmonics,
                        float inrush_ratio, float t_settle_ms, FeatureVector& out);

// FFT real de FFT_SIZE pontos sobre a corrente da janela do evento; devolve as
// magnitudes relativas das N_HARMONICS harmônicas ímpares. Usa esp-dsp se disponível;
// aqui vai uma DFT direta de referência (substituir por esp_dsp no deploy).
void computeCurrentHarmonics(const float* current_window, int n, float* harmonics_out);
