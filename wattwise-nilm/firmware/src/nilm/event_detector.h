#pragma once
#include "dsp/metrics.h"

// Um evento de comutação detectado (aparelho ligou ou desligou).
struct SwitchEvent {
  float delta_p;    // W (positivo = ligou, negativo = desligou)
  float delta_q;    // VAr
  float pf_before;
  float pf_after;
  uint32_t t_ms;
};

// Detector baseado em degrau de potência total com histerese/debounce.
// Estratégia: mantém o patamar estável atual de P_total. Quando P sai da faixa por
// EVENT_STABLE_CYCLES e assenta num novo patamar, registra a diferença como evento.
class EventDetector {
 public:
  // Alimenta métricas de um ciclo. Retorna true e preenche 'ev' quando há evento.
  bool feed(const CycleMetrics& m, SwitchEvent& ev);

 private:
  bool have_baseline_ = false;
  float base_p_ = 0, base_q_ = 0, base_pf_ = 0;  // patamar estável atual
  float cand_p_ = 0, cand_q_ = 0, cand_pf_ = 0;  // patamar candidato
  int stable_count_ = 0;
};
