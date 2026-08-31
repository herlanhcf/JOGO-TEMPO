#include "nilm/state_tracker.h"
#include <cmath>

void StateTracker::applyEvent(const SwitchEvent& ev, const Classification& cls) {
  ApplianceState& st = states_[clamp(cls.appliance_id)];
  if (ev.delta_p >= 0) {           // ligou
    st.on = true;
    st.power_w = ev.delta_p;
  } else {                          // desligou
    st.on = false;
    st.power_w = 0;
  }
  st.last_change_ms = ev.t_ms;
}

void StateTracker::integrate(uint32_t now_ms) {
  if (last_integrate_ms_ == 0) { last_integrate_ms_ = now_ms; return; }
  const float dt_h = (now_ms - last_integrate_ms_) / 3600000.0f;  // ms → horas
  last_integrate_ms_ = now_ms;
  for (auto& st : states_) {
    if (st.on) st.energy_wh += st.power_w * dt_h;
  }
}
