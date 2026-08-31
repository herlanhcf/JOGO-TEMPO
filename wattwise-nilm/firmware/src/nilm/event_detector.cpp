#include "nilm/event_detector.h"
#include "config.h"
#include <cmath>

static inline float sumQ(const CycleMetrics& m) {
  return m.q[0] + m.q[1] + m.q[2];
}
static inline float avgPF(const CycleMetrics& m) {
  return (m.pf[0] + m.pf[1] + m.pf[2]) / 3.0f;
}

bool EventDetector::feed(const CycleMetrics& m, SwitchEvent& ev) {
  const float p = m.p_total;
  const float q = sumQ(m);
  const float pf = avgPF(m);

  if (!have_baseline_) {
    base_p_ = p; base_q_ = q; base_pf_ = pf;
    have_baseline_ = true;
    return false;
  }

  // Dentro da faixa do patamar atual? então só acompanha (média lenta).
  if (fabsf(p - base_p_) < EVENT_POWER_THRESH_W) {
    base_p_ = 0.98f * base_p_ + 0.02f * p;   // leve tracking do baseline
    base_q_ = 0.98f * base_q_ + 0.02f * q;
    base_pf_ = 0.98f * base_pf_ + 0.02f * pf;
    stable_count_ = 0;
    return false;
  }

  // Saiu da faixa: estamos num possível novo patamar. Precisa se manter estável.
  if (stable_count_ == 0 || fabsf(p - cand_p_) > EVENT_POWER_THRESH_W) {
    cand_p_ = p; cand_q_ = q; cand_pf_ = pf;
    stable_count_ = 1;
    return false;
  }

  if (++stable_count_ < EVENT_STABLE_CYCLES) return false;

  // Novo patamar confirmado → emite evento com a diferença do patamar anterior.
  ev.delta_p = cand_p_ - base_p_;
  ev.delta_q = cand_q_ - base_q_;
  ev.pf_before = base_pf_;
  ev.pf_after = cand_pf_;
  ev.t_ms = m.t_ms;

  base_p_ = cand_p_; base_q_ = cand_q_; base_pf_ = cand_pf_;
  stable_count_ = 0;
  return true;
}
