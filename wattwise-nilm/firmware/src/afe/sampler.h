#pragma once
#include <cstdint>
#include "util/ring_buffer.h"

// Uma amostra sincronizada: os 3 canais de corrente + tensão, colhidos no mesmo instante
// (na prática o ADC continuous intercala; alinhamos por bloco). Valores em contagens
// brutas do ADC (0..4095), com o bias Vcc/2 ainda presente — remoção do offset e
// calibração são feitas no módulo de métricas.
struct Sample {
  uint16_t i[3];
  uint16_t v;
  uint32_t t_us;  // timestamp da amostra (esp_timer)
};

class Sampler {
 public:
  explicit Sampler(RingBuffer<Sample>& out) : out_(out) {}

  // Configura ADC1 em modo continuous (DMA) intercalando I1,I2,I3,V a ADC_SAMPLE_RATE_HZ
  // por canal. Ver esp_adc/adc_continuous.h. Retorna false se falhar.
  bool begin();

  // Task de tempo-real: lê blocos do DMA e empurra amostras alinhadas no ring buffer.
  // Deve rodar fixada no Core 0 com prioridade alta.
  static void task(void* arg);

 private:
  RingBuffer<Sample>& out_;
};
