#include "afe/sampler.h"
#include "config.h"

#include <Arduino.h>
#include <esp_adc/adc_continuous.h>
#include <esp_timer.h>

// NOTA DE IMPLEMENTAÇÃO
// --------------------
// Este é o esqueleto do caminho de amostragem. O ADC continuous do ESP32-S3 entrega
// um fluxo intercalado dos canais habilitados. A conversão exata (formato do frame,
// unidade de handle) segue esp_adc/adc_continuous.h da versão do IDF empacotada pelo
// Arduino-ESP32 em uso. Os pontos marcados TODO precisam ser fechados no bring-up com
// o hardware real (rotação de canais, ganho/atenuação e calibração).

static adc_continuous_handle_t s_handle = nullptr;

bool Sampler::begin() {
  adc_continuous_handle_cfg_t handle_cfg = {
      .max_store_buf_size = 4096,
      .conv_frame_size = 256,
  };
  if (adc_continuous_new_handle(&handle_cfg, &s_handle) != ESP_OK) return false;

  // Padrão de canais: I1, I2, I3, V (ADC1_CH0..CH3), 12 bits, atenuação 11 dB (0..3.3V).
  adc_digi_pattern_config_t pattern[4] = {};
  const adc_channel_t chans[4] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2,
                                  ADC_CHANNEL_3};
  for (int k = 0; k < 4; ++k) {
    pattern[k].atten = ADC_ATTEN_DB_11;
    pattern[k].channel = chans[k];
    pattern[k].unit = ADC_UNIT_1;
    pattern[k].bit_width = ADC_BITWIDTH_12;
  }

  adc_continuous_config_t cfg = {
      .pattern_num = 4,
      .adc_pattern = pattern,
      // 4 canais * taxa por canal:
      .sample_freq_hz = (uint32_t)ADC_SAMPLE_RATE_HZ * 4,
      .conv_mode = ADC_CONV_SINGLE_UNIT_1,
      .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
  };
  if (adc_continuous_config(s_handle, &cfg) != ESP_OK) return false;
  return adc_continuous_start(s_handle) == ESP_OK;
}

void Sampler::task(void* arg) {
  Sampler* self = static_cast<Sampler*>(arg);
  static uint8_t buf[256];
  Sample s{};
  uint8_t got_mask = 0;  // bits: quais canais do quadro atual já foram lidos

  for (;;) {
    uint32_t out_len = 0;
    esp_err_t err = adc_continuous_read(s_handle, buf, sizeof(buf), &out_len, 50);
    if (err != ESP_OK) { vTaskDelay(1); continue; }

    for (uint32_t off = 0; off < out_len; off += SOC_ADC_DIGI_RESULT_BYTES) {
      auto* p = reinterpret_cast<adc_digi_output_data_t*>(&buf[off]);
      const uint32_t ch = p->type2.channel;
      const uint16_t val = p->type2.data;

      switch (ch) {
        case ADC_CHANNEL_0: s.i[0] = val; got_mask |= 0x1; break;
        case ADC_CHANNEL_1: s.i[1] = val; got_mask |= 0x2; break;
        case ADC_CHANNEL_2: s.i[2] = val; got_mask |= 0x4; break;
        case ADC_CHANNEL_3: s.v    = val; got_mask |= 0x8; break;
        default: break;
      }
      if (got_mask == 0xF) {  // quadro completo (I1,I2,I3,V)
        s.t_us = (uint32_t)esp_timer_get_time();
        self->out_.push(s);
        got_mask = 0;
      }
    }
  }
}
