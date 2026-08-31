# Notas de esquemático (nível netlist)

Sem EDA neste repo — aqui vai a descrição para reproduzir o esquemático em KiCad.
Fabricação alvo: JLCPCB/PCBA. Ver `../docs/04-pcb.md` e `../docs/07-seguranca.md`.

## Alimentação
- Entrada de rede L/N → F1 (fusível 1A) → RV1 (MOV 275V entre L-N) → RT1 (NTC em série).
- L/N pós-proteção → **HLK-5M05** → 5V isolado.
- 5V → **U3 (TLV75733)** → 3V3 analógico limpo. 5V → (LDO/DCDC) → 3V3 digital.
- Ferrite bead entre 3V3_dig e 3V3_ana. Bulk 100µF perto do módulo Wi-Fi.

## Referência de bias (Vcc/2)
- Divisor 2×10k de 3V3_ana → 1,65V, filtrado com 10µF.
- Buffer por **U4 (MCP6002)** → trilho VREF distribuído a todos os AFEs.

## AFE de corrente (×3, um por canal)
- Conector CT (J3) → resistor de burden R_burden (22Ω, dimensionar p/ o range) entre os
  terminais do secundário.
- Um lado do burden em VREF (1,65V), o outro no nó de sinal.
- Nó de sinal → filtro RC anti-alias (ex.: 1k + 150nF ≈ 1 kHz) → pino ADC (GPIO1/2/3).
- **D1 (BAT54S)** de clamp entre o pino e 3V3/GND.

## AFE de tensão (isolado)
- Rede L/N → primário do **T1 (ZMPT101B)** (com resistor série no primário conforme
  datasheet do módulo). Secundário → burden → bias em VREF → filtro RC → ADC (GPIO4).
- T1 provê a barreira de isolação da medição de tensão.

## Saídas de automação
- GPIO14/GPIO21 → resistor → LED do **OK1 (PC817)** → transistor do opto aciona o
  **K1 (G3MB-202P SSR)** → bornes de carga isolados. Snubber RC opcional na saída SSR.

## Digital
- **DS3231** em I2C (GPIO8/9) com pull-ups 4k7 e bateria de backup (CR2032).
- **microSD** em SPI (CS=GPIO10, CLK=12, MOSI=11, MISO=13).
- **USB-C** (GPIO19/20) com **U6 (USBLC6)** de ESD.
- **WS2812B** em GPIO48; **botão** em GPIO0 com pull-up + RC de debounce.
- Header de debug: UART0 (GPIO43/44), EN, IO0, 3V3, GND.

## Barreira de isolação
- Tudo à esquerda de HLK-5M05/T1/PC817 é **lado de rede**. Nada de GND de sinal cruza.
- Slots fresados sob T1 e OK1; creepage ≥ 6,4 mm. Ver checklist em `../docs/04-pcb.md`.
