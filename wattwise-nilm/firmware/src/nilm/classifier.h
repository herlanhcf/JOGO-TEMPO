#pragma once
#include "dsp/features.h"

// Resultado da classificação de um evento.
struct Classification {
  int appliance_id;     // índice do aparelho (-1 = desconhecido)
  float confidence;     // 0..1
};

// Interface do classificador NILM. Duas implementações possíveis por baixo:
//   (A) TFLite Micro / ESP-DL — rede treinada, quantizada int8;
//   (B) kNN no espaço de features — fallback sem treino, aprende por instalação.
// O restante do firmware só depende desta interface.
class Classifier {
 public:
  virtual ~Classifier() = default;
  virtual bool begin() = 0;
  virtual Classification infer(const FeatureVector& fv) = 0;
  virtual const char* applianceName(int id) = 0;
};

// Fábrica: escolhe a implementação disponível (modelo no SD → TFLite; senão → kNN).
Classifier* createClassifier();
