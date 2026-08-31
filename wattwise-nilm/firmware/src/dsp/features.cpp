#include "dsp/features.h"
#include "config.h"
#include <cmath>

void buildFeatureVector(const SwitchEvent& ev, const float* harmonics,
                        float inrush_ratio, float t_settle_ms, FeatureVector& out) {
  for (int i = 0; i < N_FEATURES; ++i) out.f[i] = 0.0f;
  const float ds = sqrtf(ev.delta_p * ev.delta_p + ev.delta_q * ev.delta_q);
  out.f[0] = ev.delta_p;
  out.f[1] = ev.delta_q;
  out.f[2] = ds;
  out.f[3] = ev.pf_before;
  out.f[4] = ev.pf_after;
  out.f[5] = (ev.delta_p >= 0) ? 1.0f : -1.0f;
  out.f[6] = inrush_ratio;
  out.f[7] = t_settle_ms;
  for (int h = 0; h < N_HARMONICS && (8 + h) < N_FEATURES; ++h) {
    out.f[8 + h] = harmonics ? harmonics[h] : 0.0f;
  }
}

// DFT de referência (O(N*K)) — só para as harmônicas de interesse. No firmware final,
// troque por esp_dsp_fft2r (radix-2, acelerado no S3). Mantida simples e correta aqui.
void computeCurrentHarmonics(const float* x, int n, float* out) {
  // Componente fundamental = 1 ciclo dentro da janela. Assume janela cobrindo
  // FFT_SIZE amostras ~ 2 ciclos → fundamental no bin 2.
  const float fund_bin = 2.0f;
  float mag1 = 1e-6f;
  for (int h = 0; h < N_HARMONICS; ++h) {
    const float bin = fund_bin * (2 * h + 1);  // 1ª, 3ª, 5ª... (ímpares)
    float re = 0, im = 0;
    for (int t = 0; t < n; ++t) {
      const float ang = -2.0f * (float)M_PI * bin * t / n;
      re += x[t] * cosf(ang);
      im += x[t] * sinf(ang);
    }
    const float mag = sqrtf(re * re + im * im);
    if (h == 0) mag1 = (mag > 1e-6f) ? mag : 1e-6f;
    out[h] = mag / mag1;  // magnitude relativa à fundamental
  }
}
