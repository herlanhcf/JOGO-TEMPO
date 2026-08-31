# Pipeline de ML — WattWise NILM

Fluxo:

```
eventos rotulados (SD / datasets públicos)
        │  extract_features.py   (mesmas 20 features do firmware)
        ▼
features.csv
        │  train_nilm.py         (MLP int8-quantizado)
        ▼
model.tflite  +  norm_stats.h    (média/desvio p/ embarcar no classifier)
```

1. Colete eventos rotulados (modo rotulagem do firmware grava em `/dataset/*.csv` no SD),
   e/ou baixe PLAID/WHITED/REDD/UK-DALE.
2. `python extract_features.py --in dataset/ --out features.csv`
3. `python train_nilm.py --in features.csv --out model.tflite`
4. Embarque `model.tflite` (via SD ou `xxd -i`) e copie `norm_stats.h` para o firmware.

Ver `../docs/06-ml.md` para a teoria.
