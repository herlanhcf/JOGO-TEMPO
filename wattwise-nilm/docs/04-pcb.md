# 04 — Diretrizes de PCB (para fabricar/montar na China)

Placa mista com **tensão de rede** presente. As regras abaixo não são estéticas — são
de segurança e de qualidade de sinal. Fabricante alvo: JLCPCB (regras de 2 e 4 camadas).

## Isolação e regiões de rede (crítico)

- **Separe a placa em duas ilhas**: lado de **rede (HV)** e lado de **baixa tensão (LV)**.
  O HLK‑5M05 e o ZMPT101B fazem a ponte isolada entre elas.
- **Distância de isolação (creepage/clearance)**: ≥ **6,4 mm** entre HV e LV para 230 V
  (categoria de sobretensão II). Não passe trilhas de sinal sob a região de rede.
- **Slots/rasgos** (milled slots) na PCB embaixo do transformador e do opto para aumentar
  o creepage. JLCPCB aceita slots — declare no Gerber.
- Sem plano de cobre atravessando a barreira de isolação.
- Fusível + varistor (MOV) + NTC na entrada de rede. Bornes com espaçamento adequado.

## Aterramento e ruído (para o ADC valer alguma coisa)

- **Planos separados AGND e DGND**, unidos em **um único ponto** (star ground) perto do
  ADC/regulador.
- Regulador analógico (LDO baixo ruído) separado do digital, com ferrite bead entre 3V3
  digital e 3V3 analógico.
- Referência Vcc/2 (bias) com capacitor grande (10 µF) + buffer por op‑amp; distribua
  como um "trilho", não puxe de divisores individuais ruidosos.
- Trilhas dos sinais de corrente/tensão curtas, longe do módulo Wi‑Fi e do conversor
  AC‑DC (que é fonte de ruído chaveado). Blindagem/guard trace se possível.

## Módulo Wi‑Fi (ESP32‑S3‑WROOM)

- **Keep‑out da antena**: nada de cobre (nem GND) sob a antena; siga o datasheet do
  módulo. Coloque a antena na borda da placa, apontando para fora do gabinete.
- Desacoplamento farto no 3V3 do módulo (100 nF + 10 µF + 100 µF perto do pino).
- A corrente de pico de TX do Wi‑Fi é alta — trilha de alimentação larga + bulk cap.

## Camadas

- **Mínimo viável**: 2 camadas funciona, mas 4 camadas (SIG‑GND‑PWR‑SIG) melhora muito
  o ruído do analógico e o EMI do Wi‑Fi. Custo JLCPCB de 4 camadas é baixo hoje.
- Se 2 camadas: plano de GND o mais contínuo possível no bottom.

## Montagem (JLCPCB PCBA)

- Prefira peças **"Basic"** do LCSC para reduzir custo de setup; marque as "Extended"
  necessárias (ESP32‑S3, DS3231).
- O **ESP32‑S3‑WROOM** é módulo SMD montável pela JLCPCB (está no catálogo).
- **HLK‑5M05, ZMPT101B, SCT‑013, slot SD, bornes**: normalmente montados à mão / THT —
  deixe como "Do Not Populate" na PCBA e solde depois, ou peça montagem THT (mais caro).
- Gere: Gerber (RS‑274X), BOM (LCSC part #), e arquivo de posição (CPL). Confira a
  **orientação/rotação** dos componentes no visualizador da JLCPCB — erro de rotação é
  o problema nº 1 em PCBA.
- Adicione **fiducials** (3 por placa) para a montadora.
- Silk claro: marque L/N, polaridade, e um aviso ⚡ na região de rede.

## Checklist antes de enviar Gerber

- [ ] Creepage HV↔LV ≥ 6,4 mm conferido no DRC
- [ ] Keep‑out da antena respeitado
- [ ] AGND/DGND com star point único
- [ ] Fusível + MOV + NTC na entrada
- [ ] Diodos de clamp nos pinos de ADC
- [ ] Test points nos sinais analógicos e em 3V3/GND
- [ ] Rotação de todos os componentes conferida no CPL
- [ ] Furo de fixação + folga de gabinete
