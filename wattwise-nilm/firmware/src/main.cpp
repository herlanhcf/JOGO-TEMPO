// WattWise — ESP32-S3 NILM energy monitor
// Orquestra as três tarefas (sampler, analytics, network) e as filas entre elas.
//
// Este main amarra os módulos numa arquitetura funcional. Os pontos que dependem do
// hardware real (calibração do AFE, driver ADC continuous, embarque do modelo) estão
// marcados com TODO e documentados em docs/05-firmware.md e docs/roadmap.md.

#include <Arduino.h>
#include "config.h"
#include "util/ring_buffer.h"
#include "afe/sampler.h"
#include "dsp/metrics.h"
#include "dsp/features.h"
#include "nilm/event_detector.h"
#include "nilm/classifier.h"
#include "nilm/state_tracker.h"
#include "net/wifi_ota.h"
#include "net/mqtt_client.h"

// ---- Configuração de rede (mover para portal cativo / config.json no SD) ----
static const char* WIFI_SSID = "SUA_REDE";
static const char* WIFI_PASS = "SUA_SENHA";
static const char* MQTT_HOST = "192.168.0.10";
static const uint16_t MQTT_PORT = 1883;

// ---- Objetos globais compartilhados entre tarefas ----
static RingBuffer<Sample> g_samples(RING_CAPACITY);
static QueueHandle_t g_event_queue;  // eventos classificados p/ a task de rede

struct QueuedResult {
  SwitchEvent ev;
  Classification cls;
};

static Sampler g_sampler(g_samples);
static Classifier* g_classifier = nullptr;
static StateTracker g_tracker;
static MqttClient g_mqtt;

// ---------------------------------------------------------------------------
// Task ANALYTICS (Core 1): consome amostras, calcula métricas, detecta e
// classifica eventos, atualiza o rastreador de estado.
// ---------------------------------------------------------------------------
static void analyticsTask(void*) {
  MetricsAccumulator metrics;
  EventDetector detector;
  Sample s;
  CycleMetrics cm;
  uint32_t last_pub = 0;

  for (;;) {
    bool progressed = false;
    while (g_samples.pop(s)) {
      progressed = true;
      if (!metrics.feed(s, cm)) continue;  // ainda não fechou um ciclo

      // Publica potência total ~1x/s (evita floodar o MQTT a cada ciclo).
      if (cm.t_ms - last_pub > 1000) {
        last_pub = cm.t_ms;
        QueuedResult probe{};
        probe.cls.appliance_id = -2;       // -2 = "atualização de total"
        probe.ev.delta_p = cm.p_total;
        xQueueSend(g_event_queue, &probe, 0);
      }

      SwitchEvent ev;
      if (detector.feed(cm, ev)) {
        // TODO: extrair a janela de corrente do transiente do ring buffer para FFT.
        float harmonics[N_HARMONICS] = {0};
        // computeCurrentHarmonics(window, FFT_SIZE, harmonics);  // no deploy
        FeatureVector fv;
        buildFeatureVector(ev, harmonics, /*inrush*/ 0.0f, /*t_settle*/ 0.0f, fv);

        Classification cls = g_classifier->infer(fv);
        g_tracker.applyEvent(ev, cls);

        QueuedResult r{ev, cls};
        xQueueSend(g_event_queue, &r, 0);
      }
    }
    if (!progressed) vTaskDelay(1);
  }
}

// ---------------------------------------------------------------------------
// Task NETWORK (Core 0): Wi-Fi, MQTT, OTA, publicação de resultados.
// ---------------------------------------------------------------------------
static void networkTask(void*) {
  netbase::beginWifi(WIFI_SSID, WIFI_PASS);
  netbase::beginOta("wattwise");
  netbase::syncTime();
  g_mqtt.begin(MQTT_HOST, MQTT_PORT, nullptr, nullptr);

  // Anuncia os aparelhos conhecidos ao Home Assistant.
  for (int id = 0; id < 5; ++id) {
    g_mqtt.announceAppliance(id, g_classifier->applianceName(id));
  }

  QueuedResult r;
  uint32_t last_integrate = 0;
  for (;;) {
    netbase::loopOta();
    g_mqtt.loop();

    while (xQueueReceive(g_event_queue, &r, 0) == pdTRUE) {
      if (r.cls.appliance_id == -2) {          // atualização de total
        g_mqtt.publishTotal(r.ev.delta_p);
      } else {                                  // evento de aparelho
        const char* name = g_classifier->applianceName(r.cls.appliance_id);
        g_mqtt.publishAppliance(r.cls.appliance_id, name,
                                g_tracker.state(r.cls.appliance_id));
        Serial.printf("[EVENTO] %s  dP=%.1fW  conf=%.2f\n",
                      name, r.ev.delta_p, r.cls.confidence);
      }
    }

    const uint32_t now = millis();
    if (now - last_integrate > 1000) {
      last_integrate = now;
      g_tracker.integrate(now);
    }
    vTaskDelay(5);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nWattWise — NILM energy monitor (ESP32-S3)");

  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);

  g_classifier = createClassifier();
  g_classifier->begin();

  g_event_queue = xQueueCreate(32, sizeof(QueuedResult));

  if (!g_sampler.begin()) {
    Serial.println("ERRO: falha ao iniciar o ADC continuous!");
  }

  // Sampler no Core 0 com prioridade alta; analytics no Core 1; network no Core 0.
  xTaskCreatePinnedToCore(Sampler::task, "sampler", 4096, &g_sampler, 20, nullptr, 0);
  xTaskCreatePinnedToCore(analyticsTask, "analytics", 16384, nullptr, 10, nullptr, 1);
  xTaskCreatePinnedToCore(networkTask, "network", 12288, nullptr, 5, nullptr, 0);
}

void loop() {
  vTaskDelay(1000);  // todo o trabalho está nas tasks
}
