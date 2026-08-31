# WattWise — Monitor de Energia com IA na Borda (NILM)

**WattWise** é um medidor de energia *não‑invasivo* baseado em **ESP32‑S3** que faz
**desagregação de cargas (NILM — Non‑Intrusive Load Monitoring)**: com um único sensor
de corrente pinçado no quadro de distribuição, uma rede neural rodando **dentro do
microcontrolador** identifica *quais aparelhos* estão ligados a partir da assinatura
elétrica de cada um — sem precisar de um sensor por tomada.

> Um único ponto de medição → consumo por aparelho, detecção de falhas e automação.

## Por que é interessante

- **Roda ML na borda**, sem nuvem. Usa o acelerador vetorial e a PSRAM do ESP32‑S3 —
  coisa que um ESP32 clássico não faz bem. Esse é o diferencial do projeto.
- **Não‑invasivo**: sensor de corrente por indução (CT clamp), instalado sem cortar fios.
- **Útil de verdade**: consumo por aparelho em R$, detecção de anomalia (ex.: motor da
  geladeira puxando mais corrente = compressor degradando), automação por padrão de uso.
- **Exige PCB dedicada** (front‑end analógico misto + isolação de rede), o que justifica
  mandar fabricar/montar (JLCPCB/PCBA).

## O que faz

1. Amostra corrente (até 3 canais) e tensão da rede a alta taxa (~4 kHz).
2. Calcula grandezas por ciclo: potência ativa (P), reativa (Q), fator de potência,
   RMS e conteúdo harmônico (FFT).
3. **Detecta eventos** (degraus de potência = aparelho ligou/desligou).
4. **Classifica o aparelho** pela assinatura do transiente + estado estacionário
   (rede neural quantizada int8, TFLite Micro / ESP‑DL).
5. Publica por **MQTT** (Home Assistant auto‑discovery), expõe painel local e permite
   **automação** via 2 saídas a relé (ex.: desligar cargas em horário de pico).
6. Registra dados no cartão SD para retreinar/rotular aparelhos.

## Estrutura do repositório

```
wattwise-nilm/
├── docs/          Conceito, hardware, PCB, firmware, ML, segurança, roadmap
├── hardware/      Descrição de esquemático (nível netlist), pinagem, BOM
├── firmware/      Projeto PlatformIO (ESP32‑S3, Arduino/ESP‑IDF)
└── ml/            Pipeline Python: extração de features, treino e quantização
```

Comece por [`docs/01-conceito.md`](docs/01-conceito.md) e
[`docs/02-hardware.md`](docs/02-hardware.md).

## Status

Projeto de referência / ponto de partida. O firmware é um esqueleto funcional
(amostragem + DSP + MQTT + OTA prontos; o classificador entra como stub com
interface definida). Veja [`docs/roadmap.md`](docs/roadmap.md) para o que falta.

## Segurança ⚠️

Este projeto mede a **rede elétrica**. Leia [`docs/07-seguranca.md`](docs/07-seguranca.md)
**antes** de montar. Use sensores de corrente por indução (sem contato com o condutor),
isolação galvânica na medição de tensão e uma fonte AC‑DC isolada e certificada.
