#include "net/mqtt_client.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient s_wifi;
static PubSubClient s_mqtt(s_wifi);
static const char* s_user = nullptr;
static const char* s_pass = nullptr;

static String deviceId() {
  uint64_t mac = ESP.getEfuseMac();
  char buf[13];
  snprintf(buf, sizeof(buf), "%04X%08X", (uint16_t)(mac >> 32), (uint32_t)mac);
  return String("wattwise_") + buf;
}

bool MqttClient::begin(const char* host, uint16_t port, const char* user, const char* pass) {
  s_user = user; s_pass = pass;
  s_mqtt.setServer(host, port);
  s_mqtt.setBufferSize(1024);  // discovery payloads são grandes
  return reconnect();
}

bool MqttClient::reconnect() {
  if (s_mqtt.connected()) return true;
  const String id = deviceId();
  const String willTopic = String(MQTT_BASE_TOPIC) + "/" + id + "/status";
  if (s_mqtt.connect(id.c_str(), s_user, s_pass, willTopic.c_str(), 0, true, "offline")) {
    s_mqtt.publish(willTopic.c_str(), "online", true);
    return true;
  }
  return false;
}

bool MqttClient::connected() { return s_mqtt.connected(); }

void MqttClient::loop() {
  if (!s_mqtt.connected()) reconnect();
  s_mqtt.loop();
}

void MqttClient::announceAppliance(int id, const char* name) {
  const String dev = deviceId();
  // sensor de potência
  JsonDocument doc;
  doc["name"] = String(name) + " potência";
  doc["uniq_id"] = dev + "_p_" + id;
  doc["stat_t"] = String(MQTT_BASE_TOPIC) + "/" + dev + "/appliance/" + id + "/power";
  doc["unit_of_meas"] = "W";
  doc["dev_cla"] = "power";
  doc["stat_cla"] = "measurement";
  JsonObject d = doc["dev"].to<JsonObject>();
  d["ids"] = dev;
  d["name"] = "WattWise";
  d["mdl"] = "ESP32-S3 NILM";
  d["mf"] = "WattWise";
  String topic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + dev + "_p_" + id + "/config";
  String payload;
  serializeJson(doc, payload);
  s_mqtt.publish(topic.c_str(), payload.c_str(), true);
  // (energia kWh seguiria o mesmo padrão com dev_cla=energy, stat_cla=total_increasing)
}

void MqttClient::publishTotal(float watts) {
  const String t = String(MQTT_BASE_TOPIC) + "/" + deviceId() + "/total/power";
  s_mqtt.publish(t.c_str(), String(watts, 1).c_str());
}

void MqttClient::publishAppliance(int id, const char*, const ApplianceState& st) {
  const String base = String(MQTT_BASE_TOPIC) + "/" + deviceId() + "/appliance/" + id;
  s_mqtt.publish((base + "/power").c_str(), String(st.power_w, 1).c_str());
  s_mqtt.publish((base + "/energy").c_str(), String((float)(st.energy_wh / 1000.0), 3).c_str());
  s_mqtt.publish((base + "/state").c_str(), st.on ? "ON" : "OFF");
}
