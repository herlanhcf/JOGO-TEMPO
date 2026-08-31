# 05 — Arquitetura de Firmware

Framework: **PlatformIO + Arduino‑ESP32 (com ESP‑IDF por baixo)**. Escolhido pela
facilidade de MQTT/OTA e por permitir chamar APIs IDF (ADC continuous, DMA, ESP‑DL).

## Threads (FreeRTOS)

```
Core 0                              Core 1
┌────────────────────────┐         ┌──────────────────────────────┐
│ Sampler (tempo‑real)   │  ring   │ Analytics                     │
│  - ADC continuous DMA  │ buffer  │  - por ciclo: RMS,P,Q,FP,FFT  │
│  - 16 kHz agregado     │ ──────► │  - detector de eventos (ΔP)   │
│  - empurra blocos      │         │  - features → classificador   │
└────────────────────────┘         │  - rastreio de estado/energia │
                                    └───────────┬──────────────────┘
┌────────────────────────┐                      │ fila de eventos
│ Network (Core 0/1)     │ ◄────────────────────┘
│  - Wi‑Fi + MQTT        │
│  - OTA, NTP, painel web│
│  - grava SD            │
└────────────────────────┘
```

- **Sampler** tem prioridade alta e não bloqueia — só copia amostras do DMA para um
  ring buffer em PSRAM.
- **Analytics** consome blocos alinhados ao cruzamento por zero da tensão (1 ciclo de
  rede = 50/60 Hz), calcula grandezas e detecta eventos.
- **Network** é a única que fala com o mundo (MQTT, OTA, SD). Filas desacoplam tudo.

## Módulos (`firmware/src/`)

| Arquivo | Papel |
|--------|-------|
| `main.cpp` | setup, cria tasks/filas, orquestra |
| `config.h` | pinos, taxas, limiares, tópicos MQTT |
| `afe/sampler.*` | ADC continuous + DMA → ring buffer |
| `dsp/metrics.*` | RMS, potência ativa/reativa, FP por ciclo |
| `dsp/features.*` | FFT (harmônicas) + montagem do vetor de features do evento |
| `nilm/event_detector.*` | detecção de degrau de potência (liga/desliga) |
| `nilm/classifier.*` | wrapper de inferência (TFLite Micro / ESP‑DL) — **stub definido** |
| `nilm/state_tracker.*` | quais aparelhos ligados, energia acumulada por aparelho |
| `net/wifi_ota.*` | Wi‑Fi, NTP, OTA |
| `net/mqtt_client.*` | MQTT + Home Assistant auto‑discovery |
| `util/ring_buffer.h` | ring buffer SPSC lock‑free |

## Fluxo de dados de um evento

1. `sampler` entrega janela de amostras (v[n], i[n]).
2. `metrics` calcula P, Q, FP, Irms por ciclo → série temporal de P agregado.
3. `event_detector` vê ΔP > limiar estável → dispara evento com janela em torno do
   transiente.
4. `features` monta vetor: [ΔP, ΔQ, FP_antes, FP_depois, H1..H7 da corrente, inrush,
   t_acomodação, ...].
5. `classifier.infer(features)` → id do aparelho + confiança.
6. `state_tracker` atualiza estado e publica via `mqtt_client`.

## Integração com Home Assistant

Publica via MQTT Discovery (`homeassistant/sensor/wattwise_<aparelho>/config`) um sensor
de potência por aparelho + total + energia (kWh). Assim aparece automaticamente no HA
sem configuração manual.

## OTA e configuração

- OTA por HTTP/ArduinoOTA (atualização de firmware e do modelo `.tflite` separadamente).
- Configuração (Wi‑Fi, MQTT, tarifas) via portal cativo no primeiro boot (WiFiManager) e
  arquivo `config.json` no SD.
