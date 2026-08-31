#include "net/wifi_ota.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>

namespace netbase {

bool beginWifi(const char* ssid, const char* pass, uint32_t timeout_ms) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeout_ms) {
    delay(200);
  }
  return WiFi.status() == WL_CONNECTED;
}

void beginOta(const char* hostname) {
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();
}

void syncTime(const char* tz) {
  configTzTime(tz, "pool.ntp.org", "time.nist.gov");
}

void loopOta() { ArduinoOTA.handle(); }

bool wifiOk() { return WiFi.status() == WL_CONNECTED; }

}  // namespace netbase
