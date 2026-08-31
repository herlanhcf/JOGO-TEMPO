# Pinagem — ESP32‑S3‑WROOM‑1

> ADC2 é bloqueado quando o Wi‑Fi está ativo → toda medição analógica em **ADC1**.
> Evite os pinos de strapping (0, 3, 45, 46) para funções críticas em boot.

| Função | GPIO | Periférico | Notas |
|--------|------|-----------|-------|
| Corrente CH1 | GPIO1 | ADC1_CH0 | AFE corrente |
| Corrente CH2 | GPIO2 | ADC1_CH1 | AFE corrente |
| Corrente CH3 | GPIO3 | ADC1_CH2 | AFE corrente (strapping — ok como entrada) |
| Tensão rede  | GPIO4 | ADC1_CH3 | AFE tensão isolado (ZMPT101B) |
| I²C SDA | GPIO8 | I2C0 | DS3231 RTC |
| I²C SCL | GPIO9 | I2C0 | DS3231 RTC |
| SD CLK | GPIO12 | SPI/SDMMC | microSD |
| SD CMD/MOSI | GPIO11 | SPI/SDMMC | microSD |
| SD D0/MISO | GPIO13 | SPI/SDMMC | microSD |
| SD CS/D3 | GPIO10 | SPI | microSD |
| Relé 1 | GPIO14 | GPIO out | Aciona SSR 1 (via opto) |
| Relé 2 | GPIO21 | GPIO out | Aciona SSR 2 (via opto) |
| LED RGB | GPIO48 | RMT | WS2812B (padrão em muitas DevKits S3) |
| Botão | GPIO0 | GPIO in | Strapping/boot + função "rotular" (com pull‑up) |
| Mic I²S BCLK | GPIO15 | I2S | Opcional (expansão acústica) |
| Mic I²S WS | GPIO16 | I2S | Opcional |
| Mic I²S DIN | GPIO17 | I2S | Opcional |
| USB D‑ | GPIO19 | USB‑OTG | Nativo |
| USB D+ | GPIO20 | USB‑OTG | Nativo |
| UART0 TX | GPIO43 | UART | Debug/console |
| UART0 RX | GPIO44 | UART | Debug/console |

Reserve GPIO35–37 se usar PSRAM octal (N16R8) — já ocupados internamente pelo módulo.
