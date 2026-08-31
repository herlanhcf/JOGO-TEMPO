#pragma once
// WattWise — configuração central (pinos, taxas, limiares).

// ---- Rede elétrica ----
#define MAINS_FREQ_HZ        60      // 60 no Brasil (troque para 50 se aplicável)
#define ADC_SAMPLES_PER_CYCLE 66     // ~4 kHz / 60 Hz por canal
#define ADC_SAMPLE_RATE_HZ   (MAINS_FREQ_HZ * ADC_SAMPLES_PER_CYCLE) // ~3960 Hz/canal

// ---- Canais de medição ----
#define N_CURRENT_CHANNELS   3
#define PIN_ADC_I1           1       // ADC1_CH0
#define PIN_ADC_I2           2       // ADC1_CH1
#define PIN_ADC_I3           3       // ADC1_CH2
#define PIN_ADC_V            4       // ADC1_CH3

// ---- Calibração (ajustar no bring-up com carga conhecida) ----
// Fator que converte a leitura RMS do ADC (em contagens) para Amperes / Volts.
#define CAL_I_AMPS_PER_COUNT   0.0f  // TODO: calibrar por canal
#define CAL_V_VOLTS_PER_COUNT  0.0f  // TODO: calibrar
#define ADC_MIDPOINT_COUNTS    2048  // bias Vcc/2 nominal (ajustar por offset medido)

// ---- Periféricos ----
#define PIN_I2C_SDA          8
#define PIN_I2C_SCL          9
#define PIN_SD_CS            10
#define PIN_RELAY_1          14
#define PIN_RELAY_2          21
#define PIN_LED_RGB          48
#define PIN_BUTTON           0

// ---- Detecção de eventos NILM ----
#define EVENT_POWER_THRESH_W   20.0f  // degrau mínimo p/ contar como evento
#define EVENT_STABLE_CYCLES    8      // ciclos estáveis p/ confirmar novo patamar
#define EVENT_WINDOW_CYCLES    30     // janela de transiente capturada em torno do evento

// ---- Features / FFT ----
#define FFT_SIZE             128      // 2 ciclos ~ 128 amostras; potência de 2
#define N_HARMONICS          7        // H1..H7 (ímpares relevantes)

// ---- MQTT ----
#define MQTT_BASE_TOPIC      "wattwise"
#define HA_DISCOVERY_PREFIX  "homeassistant"

// ---- Buffers ----
#define RING_CAPACITY        (ADC_SAMPLES_PER_CYCLE * 64) // ~1 s por canal, em PSRAM
