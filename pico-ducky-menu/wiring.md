# Diagrama de Ligação - Pico Ducky com Menu

## Visão Geral

```
┌─────────────────┐
│   PICO 2        │
│   RP2350        │
└─────────────────┘
      ↓ USB-C
  [Máquina Alvo]
  (se comporta como teclado)

Display I2C (LCD 16x2)  ←→  Pico (GPIO 4, 5)
3 Botões              ←→  Pico (GPIO 10, 11, 12)
```

## Componentes

| Componente | Modelo | Preço aprox | Link |
|---|---|---|---|
| Pico 2 | RP2350 | R$ 40 | [Sparkfun](https://www.sparkfun.com/products/18288) |
| Display LCD | 16x2 I2C PCF8574 | R$ 30 | Mercado Livre / Aliexpress |
| Botões | Push Button 6mm | R$ 2 cada | Qualquer eletrônica |
| Resistores | 10kΩ 1/4W | R$ 0.10 cada | Qualquer eletrônica |
| Fios jumper | 22AWG dupont | R$ 5 | Qualquer eletrônica |

## Pinagem Pico 2 (RP2350)

```
    ┌─────────────────────┐
    │ PICO 2 (RP2350)     │
    │                     │
GND─┤1                 40─┤─ VBUS (5V USB)
GP0─┤2                 39─┤─ GND
GP1─┤3                 38─┤─ GND
GND─┤4                 37─┤─ 3V3 (saída)
GP2─┤5                 36─┤─ 3V3 (entrada)
GP3─┤6                 35─┤─ ADC_VREF
GND─┤7                 34─┤─ GND
GP4─┤8  ← SDA I2C      33─┤─ GP28
GP5─┤9  ← SCL I2C      32─┤─ GP27
GND─┤10                31─┤─ GND
GP6─┤11                30─┤─ GP26
GP7─┤12                29─┤─ GP25
GND─┤13                28─┤─ GND
GP8─┤14                27─┤─ GP24
GP9─┤15                26─┤─ GP23
GND─┤16                25─┤─ GND
GP10─┤17 ← BTN UP      24─┤─ GP22
GP11─┤18 ← BTN SEL     23─┤─ GP21
GND─┤19                22─┤─ GND
GP12─┤20 ← BTN DOWN    21─┤─ GP20
    └─────────────────────┘
         (vista de cima)
```

## Ligação Display I2C (LCD 16x2 com PCF8574)

**Display tem 4 pinos:**

| Display | Cor típico | Pico 2 | Descrição |
|---------|-----------|---------|-----------|
| GND | Preto | GND (p. 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34) | Terra |
| VCC | Vermelho | 5V (VBUS, p. 40) | Alimentação 5V |
| SDA | Verde | GP4 (p. 8) | I2C Data |
| SCL | Amarelo | GP5 (p. 9) | I2C Clock |

**Esquema ASCII:**

```
┌────────────────┐
│ Display LCD    │
│  16x2 I2C      │
└────────────────┘
     ↓ 4 pinos
   
   GND  ←─→  [10kΩ resistor] ←─→  GND (Pico)
   VCC  ←─→  5V (USB/VBUS do Pico)
   SDA  ←─→  GP4 (Pico p. 8)
   SCL  ←─→  GP5 (Pico p. 9)
```

## Ligação Botões (3x Push Button)

Cada botão é ligado entre um pino GPIO e GND. O resistor pull-down mantém o pino em LOW quando não há pressão.

**Botão UP:**

```
   [+3.3V] ← (opcional, se quiser com pull-up)
      ↓
   Button
      ↓
     10kΩ   ← pull-down resistor
      ↓
   GP10 (Pico p. 17)
      ↓
    [GND]
```

**Layout dos 3 botões:**

| Botão | GPIO | Pico Pino | Função |
|-------|------|-----------|--------|
| UP | GP10 | 17 | Navega para cima no menu |
| SELECT | GP11 | 18 | Executa payload selecionado |
| DOWN | GP12 | 20 | Navega para baixo |

**Esquema completo dos botões:**

```
Botão UP:        Botão SELECT:    Botão DOWN:
┌─────────┐      ┌─────────┐      ┌─────────┐
│  Button │      │  Button │      │  Button │
└────┬────┘      └────┬────┘      └────┬────┘
     │                │                │
    10kΩ             10kΩ             10kΩ
     │                │                │
     ├────────────────┼────────────────┤
                      │
                     GND (Pico p. 19 ou 22, 25, etc)

         ↓ saída dos botões:

    GP10 (p. 17)  GP11 (p. 18)  GP12 (p. 20)
         ↓              ↓              ↓
       Pico 2
```

## Resumo de Ligações (Tabela Completa)

| Componente | Pino | Pico 2 Pino | Observação |
|---|---|---|---|
| **Display LCD** |
| GND | - | 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34 | Terra |
| VCC | - | 40 (VBUS, 5V) | Alimentação |
| SDA | - | 8 (GP4) | I2C Data |
| SCL | - | 9 (GP5) | I2C Clock |
| **Botão UP** |
| Pressão | - | 17 (GP10) + GND via 10kΩ | Pull-down |
| **Botão SELECT** |
| Pressão | - | 18 (GP11) + GND via 10kΩ | Pull-down |
| **Botão DOWN** |
| Pressão | - | 20 (GP12) + GND via 10kΩ | Pull-down |
| **USB** |
| Data + Power | - | USB-C | Para máquina alvo (HID) |

## Protoboad / Breadboard (Opcional)

Se usar breadboard, o layout é assim:

```
        Pico 2 em pé no meio da breadboard
        
        ┌─────────────┐
        │  Pico 2     │
        └─────────────┘
              ↓
        
   [Display I2C com 4 fios na esquerda]
        
   [3 Botões + resistores na direita]
```

## Teste de Ligação

Antes de plugar o Pico na máquina alvo, teste com um multímetro:

1. **I2C:**
   - SDA (GP4) e SCL (GP5) devem estar em HIGH quando idle (~3.3V)
   - Display deve aparecer com endereço 0x27 (ou 0x3F em alguns modelos)

2. **Botões:**
   - Pressione e veja tensão em GP10/11/12 ir de LOW (0V) pra HIGH (~3.3V)
   - Verifique debounce (sem ruído elétrico)

3. **USB:**
   - Pico deve ser reconhecido como "USB HID Keyboard" quando plugado

## Troubleshooting

| Problema | Causa | Solução |
|---|---|---|
| Display não mostra nada | I2C não está ligado | Revise pinos 4 e 5 (SDA/SCL) |
| Display mostra lixo | Endereço I2C errado | Mude em code.py: `address=0x3F` |
| Botões não respondem | Resistor pull-down faltando ou com mau contato | Revise resistores de 10kΩ e ligações |
| Pico não é reconhecido como teclado | Cabo USB mal encaixado | Use USB-C de qualidade, teste em outro porto |
| Keystroke injection não funciona | CircuitPython versão antiga | Atualize pro latest RP2350 |

## Dicas de Montagem

1. **Use fios de cores diferentes:**
   - Vermelho = 5V/3.3V
   - Preto = GND
   - Verde = SDA
   - Amarelo = SCL
   - Azul = Sinais dos botões

2. **Soldagem vs Breadboard:**
   - Breadboard = fácil, testável, mas instável em movimento
   - Soldagem = robusto, compacto, ideal pra apresentação

3. **Proteção:**
   - Use luva anti-estática
   - Evite descarga eletrostática (ESD)
   - Não retirar/plugar componentes com Pico ligado

---

**Próximo passo:** Copiar firmware (`code.py`, `payloads.py`, `display_ui.py`) pro Pico e testar!
