#pragma once
#include <cstdint>
#include "nilm/state_tracker.h"

// Cliente MQTT fino sobre PubSubClient, com auto-discovery do Home Assistant.
// Publica: potência total, potência e energia por aparelho, e recebe comandos de relé.
class MqttClient {
 public:
  bool begin(const char* host, uint16_t port, const char* user, const char* pass);
  void loop();  // chamar no laço da task de rede

  // Publica o discovery de um aparelho (cria os sensores no HA automaticamente).
  void announceAppliance(int id, const char* name);

  void publishTotal(float watts);
  void publishAppliance(int id, const char* name, const ApplianceState& st);
  bool connected();

 private:
  bool reconnect();
};
