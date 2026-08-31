#pragma once
#include <cstdint>

// Conectividade base: Wi-Fi, sincronização de hora (NTP) e OTA (ArduinoOTA).
namespace netbase {
bool beginWifi(const char* ssid, const char* pass, uint32_t timeout_ms = 20000);
void beginOta(const char* hostname);
void syncTime(const char* tz = "BRT3");  // fuso do Brasil por padrão
void loopOta();                          // chamar na task de rede
bool wifiOk();
}  // namespace netbase
