#pragma once
#include <cstdint>
#include "nilm/classifier.h"
#include "nilm/event_detector.h"

// Mantém o estado de cada aparelho (ligado/desligado), a potência atribuída e a energia
// acumulada (kWh). Atualizado a cada evento classificado.
struct ApplianceState {
  bool on = false;
  float power_w = 0;       // potência estimada enquanto ligado
  double energy_wh = 0;    // energia acumulada
  uint32_t last_change_ms = 0;
};

class StateTracker {
 public:
  // Aplica um evento já classificado. delta_p do evento é atribuído ao aparelho.
  void applyEvent(const SwitchEvent& ev, const Classification& cls);

  // Integra energia com o tempo decorrido (chamar periodicamente, ex.: 1x/s).
  void integrate(uint32_t now_ms);

  const ApplianceState& state(int id) const { return states_[clamp(id)]; }
  static constexpr int kMaxAppliances = 8;

 private:
  static int clamp(int id) {
    return (id >= 0 && id < kMaxAppliances) ? id : (kMaxAppliances - 1);
  }
  ApplianceState states_[kMaxAppliances];
  uint32_t last_integrate_ms_ = 0;
};
