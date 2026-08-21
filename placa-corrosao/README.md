# Placa – Controle ESP32 + Dual NRF24L01 (fabricação caseira por corrosão)

Documentação inicial para transformar o protótipo em jumpers (foto/diagrama de
referência) em uma PCB fabricada em casa pelo método de **corrosão**
(transferência de toner + percloreto de ferro) em **placa de fenolite**,
projetada no **Altium Designer**.

> Este é o ponto de partida ("como começo"): documenta o pinout, a lista de
> materiais e, principalmente, as regras de projeto para que a placa saia
> corroível sem falhas. Ainda não é o `.PcbDoc` final — isso é feito no
> Altium na sua máquina.

## 1. Visão geral do circuito

- **ESP32 DevKit** (38 pinos, módulo WROOM-32, USB micro, botões EN/BOOT de bordo)
- **Display OLED** (I2C, 4 pinos: GND, VCC, SCL, SDA)
- **5x botões táteis** (entradas digitais)
- **2x módulos NRF24L01+PA+LNA** (com antena externa), num barramento SPI
  compartilhado, cada um com CE/CSN próprios

## 2. Lista de materiais (BOM)

| Item | Qtd | Observação |
|---|---|---|
| ESP32 DevKit 38 pinos | 1 | usar soquete/header fêmea na placa, não soldar direto |
| Display OLED 0.96" I2C (4 pinos) | 1 | idem — header fêmea |
| Módulo NRF24L01+PA+LNA c/ antena | 2 | idem — header fêmea |
| Botão tátil 6x6mm | 5 | THT, fácil de corroer |
| Resistor 10kΩ | 5 (ou 2–3) | pull-down/pull-up externo dos botões (ver §4) |
| Capacitor eletrolítico 10–47µF | 2 | um por módulo NRF24 (desacoplamento local) |
| Capacitor cerâmico 100nF | 3–4 | um por VCC de cada módulo + regulador |
| Regulador 3.3V dedicado (ex.: AMS1117-3.3 ou AP2112-3.3) | 1 | **não alimentar os 2 NRF24 pelo pino 3V3 do ESP32** (ver §4) |
| Placa de fenolite virgem (face simples) | 1 | ver §6 |
| Percloreto de ferro (FeCl₃) | — | corrosão |
| Papel fotográfico glossy (para toner transfer) | — | ou papel de revista |

## 3. Tabela de conexões (pinout)

⚠️ **Leitura do diagrama enviado — confirme fisicamente (continuidade/multímetro)
antes de furar e corroer.** Os NRF24L01 **não toleram 5V** em VCC; um erro de
pino aqui queima o módulo.

### OLED (I2C)

| OLED | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

### NRF24L01 (barramento SPI compartilhado entre os 2 módulos)

| Sinal | ESP32 | NRF24 #1 | NRF24 #2 |
|---|---|---|---|
| GND | GND | GND | GND |
| VCC (3.3V regulado, ver §4) | — | VCC | VCC |
| SCK | GPIO 18 | SCK | SCK |
| MOSI | GPIO 23 | MOSI | MOSI |
| MISO | GPIO 19 | MISO | MISO |
| CE | — | GPIO 4 | GPIO 16 |
| CSN | — | GPIO 5 | GPIO 17 |
| IRQ (opcional) | — | GPIO 27 (ou não conectar) | GPIO 26 (ou não conectar) |

### Botões (5x)

| Botão | ESP32 GPIO | Observação |
|---|---|---|
| 1 | GPIO 34 | sem pull-up interno — precisa resistor externo |
| 2 | GPIO 35 | sem pull-up interno — precisa resistor externo |
| 3 | GPIO 32 | pull-up interno via firmware OK |
| 4 | GPIO 33 | pull-up interno via firmware OK |
| 5 | GPIO 25 | pull-up interno via firmware OK |

## 4. Atenção especial: alimentação

Dois módulos **NRF24L01+PA+LNA** podem consumir picos de ~120–170 mA **cada**
durante a transmissão. O regulador 3.3V embutido no DevKit (alimentado via
USB) frequentemente não aguenta esse pico somado ao ESP32 + OLED, causando
resets aleatórios ("brownout").

Recomendações para a placa:

1. Alimentar os dois módulos NRF24 a partir de um **regulador 3.3V dedicado**
   (separado do 3V3 do ESP32), com capacitor de 10–47µF + 100nF bem próximos
   de cada VCC do módulo.
2. Se possível, alimentar a placa toda por uma fonte externa de 5V (não só
   pela USB do PC) com capacidade de pelo menos 1A.
3. Rotear o plano de GND o mais largo/curto possível entre os módulos e o
   regulador — em placa de face simples, use uma trilha de GND grossa (ver §5).

## 5. Regras de projeto para corrosão (Design Rules no Altium)

Fabricação caseira por corrosão é bem mais tolerante que fábrica, mas exige
margens generosas. Configurar em **Design ▸ Rules** no Altium:

| Regra | Valor sugerido | Motivo |
|---|---|---|
| Trilha mínima (Width) | 0.5 mm (20 mil) — 0.4 mm no mínimo | trilhas finas sub-corroem/quebram no FeCl₃ |
| Clearance (espaçamento) | 0.4–0.5 mm (16–20 mil) | evita curto por corrosão incompleta |
| Anel de cobre em furo (pad) | o maior possível, THT | facilita furar sem perder a ilha |
| Camadas | **face simples (1 layer)** | sem furo metalizado em casa — evite 2 camadas a menos que use jumpers/wire para as vias |
| Componentes | somente THT (headers fêmea) | não faça footprint SMD à mão neste primeiro projeto |
| Silkscreen | opcional, mas útil para conferência (não corrói, é só referência visual) |

Outras dicas específicas do processo:

- Use **headers fêmea** para ESP32, OLED e os 2 módulos NRF24 em vez de
  soldar os módulos direto — assim você pode reposicionar/trocar peça
  queimada sem re-soldar a placa toda.
- Evite **planos de cobre (pour) muito grandes**; prefira trilhas de GND mais
  largas (ex.: 1.0–1.5 mm) só onde precisa de baixa impedância (perto dos
  NRF24). Um pour grande demora mais para corroer por igual e aumenta risco
  de "ilhas" de cobre não removidas.
- Mantenha ao menos **1.5–2 mm de borda** sem cobre ao redor da placa.
- Espelhe o layout (mirror) antes de imprimir, conforme o lado que vai ferro
  de passar o toner (regra padrão de transferência de toner).

## 6. Processo de fabricação (toner transfer + percloreto de ferro)

1. Corte a placa de fenolite no tamanho final + margem de ~5mm.
2. Lixe levemente a face de cobre (lã de aço fina) e limpe com álcool
   isopropílico — sem gordura, sem digital.
3. Imprima o layout **espelhado** em impressora laser, em papel fotográfico
   glossy (ou papel de revista), só a camada de cobre (Bottom Layer) em
   preto e branco, 100% preto.
4. Transfira com ferro de passar quente (~10–15s pré-aquecimento na placa,
   depois 3–5 min de ferro com pressão firme e uniforme).
5. Deixe esfriar, mergulhe em água morna e remova o papel com cuidado.
   Retoque falhas com caneta de retroprojetor/permanente.
7. Corroa em percloreto de ferro diluído (seguir instruções do fabricante),
   com agitação leve, até dissolver todo o cobre exposto.
8. Lave bem, remova o toner remanescente com acetona/lixa fina.
9. Fure os pads (broca 0.8–1.0mm para a maioria, conferir footprint de cada
   header) e estanhe as trilhas.
10. Solde os headers fêmea, monte os módulos e faça o teste de continuidade
    ponto a ponto **antes** de energizar.

⚠️ Percloreto de ferro mancha e corrói metal/pia — use luvas, óculos,
recipiente plástico, e descarte conforme normas locais (não jogar na pia/rede
de esgoto).

## 7. Checklist antes de corroer

- [ ] Conferir cada pino da tabela do §3 contra o diagrama original / módulos físicos
- [ ] Rodar DRC no Altium com as regras do §5 sem erros
- [ ] Conferir que não há trilha < 0.4mm nem clearance < 0.4mm
- [ ] Conferir footprints dos headers fêmea (passo 2.54mm padrão)
- [ ] Imprimir teste em papel comum e sobrepor nos módulos reais para
      conferir alinhamento dos furos antes de gastar fenolite

## 8. Próximos passos

- [ ] Montar o esquemático no Altium usando a tabela do §3 como referência
- [ ] Rotear o layout físico seguindo as regras do §5
- [ ] Gerar o Bottom Layer para impressão (espelhado)
- [ ] Fabricar um protótipo de teste (ex.: só a alimentação + 1 NRF24) antes
      da placa completa, para validar o processo de corrosão
