#include "nilm/classifier.h"
#include "config.h"
#include <Arduino.h>
#include <cmath>
#include <vector>

// -----------------------------------------------------------------------------
// Implementação de referência: kNN (k=1) no espaço de features normalizado.
// Serve como fallback funcional e como modo de "aprendizado por instalação":
// o usuário rotula alguns eventos (modo rotulagem) e eles viram protótipos.
// Quando o modelo TFLite estiver pronto, uma segunda implementação é plugada aqui
// e a fábrica passa a preferi-la se houver /model.tflite no SD.
// -----------------------------------------------------------------------------
namespace {

struct Prototype {
  FeatureVector fv;
  int appliance_id;
};

// Normalização por feature (média/desvio estimados no dataset de treino).
// TODO: preencher com estatísticas reais exportadas por ml/train_nilm.py.
static const float kMean[N_FEATURES] = {0};
static const float kStd[N_FEATURES]  = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

class KnnClassifier : public Classifier {
 public:
  bool begin() override {
    // TODO: carregar protótipos rotulados de flash/SD (/dataset/prototypes.bin).
    return true;
  }

  void addPrototype(const FeatureVector& fv, int id) {
    protos_.push_back({fv, id});
  }

  Classification infer(const FeatureVector& fv) override {
    if (protos_.empty()) return {-1, 0.0f};
    float best = INFINITY, second = INFINITY;
    int best_id = -1;
    for (const auto& p : protos_) {
      const float d = dist(fv, p.fv);
      if (d < best) { second = best; best = d; best_id = p.appliance_id; }
      else if (d < second) { second = d; }
    }
    // confiança heurística pela separação entre 1º e 2º vizinho
    const float conf = (second > 0) ? (second - best) / (second + best + 1e-6f) : 0.5f;
    return {best_id, fmaxf(0.0f, fminf(1.0f, conf))};
  }

  const char* applianceName(int id) override {
    static const char* kNames[] = {"geladeira", "chuveiro", "ar-condicionado",
                                    "micro-ondas", "bomba", "desconhecido"};
    const int n = sizeof(kNames) / sizeof(kNames[0]);
    return (id >= 0 && id < n - 1) ? kNames[id] : kNames[n - 1];
  }

 private:
  static float norm(float x, int i) { return (x - kMean[i]) / (kStd[i] + 1e-6f); }
  static float dist(const FeatureVector& a, const FeatureVector& b) {
    float s = 0;
    for (int i = 0; i < N_FEATURES; ++i) {
      const float d = norm(a.f[i], i) - norm(b.f[i], i);
      s += d * d;
    }
    return sqrtf(s);
  }
  std::vector<Prototype> protos_;
};

}  // namespace

Classifier* createClassifier() {
  // TODO: se existir /model.tflite no SD, instanciar TFLiteClassifier em vez do kNN.
  return new KnnClassifier();
}
