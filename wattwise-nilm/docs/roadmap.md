# Roadmap

## Fase 0 — Design (este repositório)
- [x] Conceito e teoria de operação (NILM na borda)
- [x] Arquitetura de hardware + pinagem + BOM
- [x] Diretrizes de PCB e segurança
- [x] Esqueleto de firmware (amostragem, DSP, MQTT, OTA, estados)
- [x] Pipeline de ML (features, treino, quantização)

## Fase 1 — Hardware
- [ ] Esquemático em KiCad a partir de `hardware/esquematico-notas.md`
- [ ] Layout com barreira de isolação e keep‑out de antena
- [ ] Rev A: pedir 5 PCBs + PCBA na JLCPCB
- [ ] Bring‑up seguro (ver `07-seguranca.md`)

## Fase 2 — Firmware base
- [ ] Calibração do AFE (ganho/offset por canal) com carga conhecida
- [ ] Amostragem DMA estável a 4 kHz/canal, cálculo de P/Q/FP/RMS validado vs. multímetro
- [ ] FFT e extração de harmônicas
- [ ] Detector de eventos (ΔP) com histerese e debounce
- [ ] MQTT + Home Assistant discovery + OTA

## Fase 3 — Coleta e ML
- [ ] Modo "rotular aparelho" (botão + app) grava eventos rotulados no SD
- [ ] Coletar dataset por instalação; treinar em `ml/train_nilm.py`
- [ ] Quantizar int8 e embarcar (TFLite Micro ou ESP‑DL)
- [ ] Rastreamento de estado + energia por aparelho

## Fase 4 — Recursos avançados
- [ ] Detecção de anomalia por aparelho (deriva de assinatura → alerta de falha)
- [ ] Automação por regras/padrão (desligar carga em pico tarifário)
- [ ] Aprendizado incremental on‑device (kNN no espaço P‑Q como fallback do NN)
- [ ] Expansão acústica (manutenção preditiva) via mic I²S
