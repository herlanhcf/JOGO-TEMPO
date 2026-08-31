# 02 — Arquitetura de Hardware

Placa mista sinal‑analógico + digital com isolação de rede. Projetada pensando em
fabricação e montagem na JLCPCB (peças no catálogo LCSC sempre que possível).

## Núcleo

| Bloco | Componente | Observações |
|------|------------|-------------|
| MCU | **ESP32‑S3‑WROOM‑1‑N16R8** | 16 MB flash, 8 MB PSRAM (octal). PSRAM é essencial p/ ML. |
| Alimentação | **HLK‑5M05** (AC‑DC isolado 5 V / 5 W) | Alimenta a placa direto da rede, isolado. Alternativa: USB‑C. |
| Regulador | AMS1117‑3.3 ou (melhor) **TLV75733** LDO baixo ruído | 3V3 limpo para o analógico. Separe planos AGND/DGND. |
| Ref. ADC | Divisor Vcc/2 com buffer (TL072/MCP6002) | Nível DC de referência dos sinais AC. |

## Front‑end de corrente (até 3 canais)

Sensor: **SCT‑013‑000** (CT clamp *tipo corrente*, 100 A : 50 mA) — não‑invasivo.

Por canal:
1. **Resistor de burden** converte a corrente do secundário em tensão (ex.: 22 Ω →
   ~±1,6 V de pico em fundo de escala; dimensione para o range desejado).
2. **Bias em Vcc/2** (dois resistores + capacitor) para centrar o AC em 1,65 V,
   cabendo no ADC 0–3,3 V.
3. **Filtro anti‑alias RC** (fc ≈ 1 kHz) antes do ADC.
4. Diodos de clamp (BAT54S) para proteção do pino do ADC.

> Alternativa de maior precisão: **CT tipo tensão** (SCT‑013‑030, saída 0–1 V) dispensa
> burden. Documentado nas notas de esquemático.

## Front‑end de tensão (1 canal, isolado)

Sensor: **ZMPT101B** (transformador de tensão de precisão) — dá isolação galvânica e
referência de fase para calcular P, Q e fator de potência corretamente.

- Módulo com trimpot de ganho ou versão discreta (transformador + burden + bias Vcc/2).
- Usado para: (a) potência **real** = média de v·i; (b) potência reativa via deslocamento
  de 90°; (c) alinhamento temporal e detecção de cruzamento por zero (sincronismo).

## Digital / periféricos

| Bloco | Componente | Uso |
|------|-----------|-----|
| Relógio | **DS3231** (I²C) | Timestamp de eventos e energia por horário/tarifa. |
| Armazenamento | Slot **microSD** (SPI ou SDMMC) | Log de eventos e datasets p/ retreino. |
| Saídas de automação | 2× **SSR** (relé estado sólido) ou relé + opto (PC817) + driver | Desligar cargas. Bornes parafuso, isolados. |
| Indicação | **WS2812B** RGB + 1 botão | Status e função de "rotular aparelho". |
| Áudio (opcional) | MEMS I²S (INMP441) | Expansão: manutenção preditiva acústica. |
| USB | USB‑C nativo do S3 (com ESD, ex. USBLC6) | Gravação/CDC/logs. |
| Depuração | Header 6 pinos (UART0 + EN + IO0) + tag‑connect JTAG | Bring‑up. |

## Mapa de ADC (ESP32‑S3)

Use **ADC1** (ADC2 conflita com Wi‑Fi). Sugestão de canais:

| Sinal | GPIO | Canal ADC1 |
|------|------|-----------|
| Corrente CH1 | GPIO1 | ADC1_CH0 |
| Corrente CH2 | GPIO2 | ADC1_CH1 |
| Corrente CH3 | GPIO3 | ADC1_CH2 |
| Tensão | GPIO4 | ADC1_CH3 |

Amostragem contínua por **DMA (I2S/ADC continuous)** intercalando os 4 canais a
~4 kHz efetivos por canal (16 kHz agregado). Ver `firmware/src/afe/sampler.*`.

Pinagem completa em [`../hardware/pinmap.md`](../hardware/pinmap.md).

## Estimativa de custo (protótipo, ordem de grandeza)

| Item | ~USD |
|------|------|
| ESP32‑S3‑WROOM‑1‑N16R8 | 4–5 |
| HLK‑5M05 | 2–3 |
| 3× SCT‑013 (comprados à parte) | 8–15 |
| ZMPT101B / DS3231 / SD / passivos | 6–10 |
| PCB (5 un., JLCPCB) | ~5 total |
| **Placa montada (sem CTs)** | **~20–30/un.** em baixa quantidade |

Ver BOM detalhada em [`../hardware/bom.csv`](../hardware/bom.csv).
