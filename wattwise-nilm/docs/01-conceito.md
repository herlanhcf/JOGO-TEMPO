# 01 — Conceito e teoria de operação

## O problema

Medidores de energia comuns mostram só o total. Para saber "quanto minha geladeira
gasta" você precisaria de um plug‑meter por aparelho — caro, chato e limitado a
tomadas. **NILM** resolve isso: a partir de *um* ponto de medição no quadro geral,
inferir o consumo de cada carga.

## A ideia central

Cada aparelho tem uma **assinatura elétrica**:

- **Estado estacionário**: um par (P, Q) característico — potência ativa e reativa.
  Uma resistência (chuveiro) é quase pura potência ativa, fator de potência ~1.
  Um motor (geladeira, bomba) tem parcela reativa grande e FP baixo.
- **Transiente de partida**: a forma como a corrente sobe ao ligar. Um motor tem
  pico de partida (inrush); uma fonte chaveada tem outra assinatura; uma carga
  resistiva sobe em degrau limpo.
- **Conteúdo harmônico**: cargas não‑lineares (fontes, LEDs, inversores) injetam
  harmônicas características (3ª, 5ª, 7ª…). Isso separa aparelhos com P/Q parecidos.

O WattWise faz **NILM baseado em eventos**:

```
Corrente total  ──► detecção de evento (ΔP)  ──► extração de features  ──► classificador  ──► "geladeira LIGOU"
                    (aparelho ligou/desligou)     (P, Q, FP, harmônicas,    (rede neural
                                                   forma do transiente)      int8 na borda)
```

1. **Detecção de evento**: monitora a potência agregada. Um degrau súbito acima de um
   limiar (ex.: 20 W) e estável por N ciclos = evento de comutação.
2. **Extração de features** na janela do evento: ΔP, ΔQ, FP antes/depois, magnitude das
   primeiras harmônicas ímpares, tempo de acomodação, pico de inrush.
3. **Classificação**: rede neural pequena (MLP ou 1D‑CNN) rotula o vetor de features
   com um aparelho conhecido. Aparelhos são cadastrados numa fase de aprendizado.
4. **Rastreamento de estado**: mantém a lista do que está ligado; integra energia por
   aparelho ao longo do tempo.

## Por que ESP32‑S3 especificamente

- **Instruções vetoriais (SIMD)** aceleram FFT e a inferência quantizada (via ESP‑DL /
  ESP‑NN). Um ESP32 original faria, mas o S3 é ~5–10× mais rápido nessas cargas.
- **PSRAM (8 MB)**: buffers de janela de forma de onda, histórico de eventos e o modelo
  cabem com folga.
- **USB nativo**: gravação/CDC/telemetria sem chip UART extra.
- **ADC decente + DMA** para amostragem contínua sincronizada com a rede.

## Limitações honestas do NILM (para calibrar expectativa)

- Cargas de baixa potência (carregadores) e cargas simultâneas com assinaturas
  parecidas são difíceis de separar. NILM comercial acerta bem os "grandes" (chuveiro,
  ar, geladeira, forno) e erra os "pequenos".
- Precisa de fase de aprendizado/rotulagem por instalação. Não é "plug and know".
- Este projeto foca no caso residencial monofásico/bifásico. Trifásico industrial é
  extensão (o hardware já prevê 3 canais de corrente).

## Diagrama de blocos do sistema

```
   Rede elétrica
        │
   ┌────┴─────────────────────────────────────────────┐
   │ Quadro de distribuição                            │
   │   CT clamp (I) ──┐        derivação de tensão ──┐  │
   └──────────────────┼───────────────────────────┼──┘
                      │                           │
             ┌────────▼──────────┐    ┌───────────▼───────────┐
             │ AFE de corrente   │    │ AFE de tensão isolado │
             │ burden + bias +   │    │ ZMPT101B + bias +     │
             │ filtro anti‑alias │    │ filtro                │
             └────────┬──────────┘    └───────────┬───────────┘
                      │  ADC1 (DMA, ~4kHz)         │
                   ┌──▼────────────────────────────▼──┐
                   │           ESP32‑S3               │
                   │  DSP (RMS,P,Q,FFT) → eventos →   │
                   │  features → NN int8 → estados    │
                   │  MQTT / OTA / SD / painel web     │
                   └──┬───────────────┬───────────┬───┘
                  Wi‑Fi/BLE      2× relé (SSR)   SD + RTC
                      │           (automação)
              Home Assistant / app
```
