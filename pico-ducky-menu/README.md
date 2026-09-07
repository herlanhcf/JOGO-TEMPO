# Pico Ducky Com Menu Interativo

**Transforme seu Raspberry Pi Pico 2 numa caneta USB BadUSB com menu selecionável.**

Um dispositivo compacto que:
- ✅ Funciona como teclado USB (HID)
- ✅ Permite selecionar qual payload executar via 3 botões
- ✅ Exibe status em display de LCD/OLED
- ✅ Suporta DuckyScript (.dd files)
- ✅ Sem dependência de PC durante o ataque

## Hardware Necessário

- **Raspberry Pi Pico 2** (RP2350)
- **Display LCD 16x2 I2C** (ou OLED SSD1306 128x64)
- **3 botões de pressão** (push buttons)
- **3 resistores 10kΩ** (pull-down)
- **Fios jumper** (macho-fêmea)
- **Cabo USB-C** (alimentação e conexão HID)

## Esquema de Ligação

```
PICO 2                 DISPLAY I2C        BOTÕES
---------              -----------        -------
GPIO 4 (SDA) ---------> SDA (5)
GPIO 5 (SCL) ---------> SCL (6)
GND ----------+-------> GND (9)
3V3 ----------+-------> VCC (4)

GPIO 10 --[10kΩ]--> GND  (Botão UP)
GPIO 11 --[10kΩ]--> GND  (Botão SELECT)
GPIO 12 --[10kΩ]--> GND  (Botão DOWN)
```

## Instalação

### 1. Preparar o Pico 2

```bash
# 1. Baixe CircuitPython pro RP2350 em:
# https://circuitpython.org/board/raspberry_pi_pico2/

# 2. Segure o botão BOOTSEL do Pico e plugue via USB
# 3. Arraste o arquivo .uf2 pra unidade que aparecer
# 4. Pico reinicia automaticamente com CircuitPython
```

### 2. Instalar Bibliotecas

Copie as pastas de `lib/` pra pasta `/lib` do Pico (via USB):
```
/lib/
  ├── adafruit_hid/
  ├── adafruit_bus_device/
  └── adafruit_circuitpython_lcd/
```

Links pra baixar:
- [Adafruit HID](https://github.com/adafruit/Adafruit_CircuitPython_HID)
- [Adafruit Bus Device](https://github.com/adafruit/Adafruit_CircuitPython_BusDevice)
- [LCD 16x2 I2C](https://github.com/adafruit/Adafruit_CircuitPython_CharLCD)

### 3. Copiar Firmware

```bash
# No Pico, via USB, copie:
code.py                 # Firmware principal
payloads.py           # Gerenciador de payloads
display_ui.py         # Controle do display
payloads/             # Pasta com .dd files
  ├── demo.dd
  ├── notepad.dd
  └── (seus payloads)
```

## DuckyScript Syntax

Crie seus payloads em `payloads/*.dd`. Exemplo `payloads/demo.dd`:

```
# Abrir Bloco de Notas no Windows
DELAY 500
GUI r
DELAY 300
STRING notepad
ENTER
DELAY 500
STRING Sua maquina foi comprometida!
DELAY 1000
```

### Comandos Suportados

| Comando | Efeito |
|---------|--------|
| `STRING texto` | Digita o texto |
| `ENTER` | Pressiona Enter |
| `SPACE` | Espaço |
| `TAB` | Tabulação |
| `GUI r` | Windows + R (cmd) |
| `GUI d` | Minimizar tudo (desktop) |
| `DELAY 500` | Aguarda 500ms |
| `CTRL a` | Ctrl+A (seleciona tudo) |
| `ALT TAB` | Alt+Tab |

## Como Usar

1. **Plugue o Pico** na máquina alvo via USB-C
2. **Seus 3 botões** aparecem no display:
   - **UP/DOWN** → Navega entre payloads listados
   - **SELECT** → Executa o payload selecionado
3. **O payload roda** — o Pico digita o código como se fosse um teclado
4. **Indicador visual** mostra status (executando, concluído)

## Estrutura de Diretórios

```
pico-ducky-menu/
├── README.md                 (este arquivo)
├── wiring.md                 (diagrama detalhado)
├── code.py                   (firmware principal)
├── payloads.py              (gerenciador de payloads)
├── display_ui.py            (controle do display I2C)
├── payloads/
│   ├── demo.dd              (payload de demonstração)
│   ├── notepad.dd           (abre Bloco de Notas)
│   └── (adicione seus)
└── lib/
    ├── adafruit_hid/
    ├── adafruit_bus_device/
    └── adafruit_circuitpython_lcd/
```

## Aviso Legal

⚠️ **AVISO IMPORTANTE PARA O TCC:**
- Use este projeto **apenas em sistemas que você controla e tem permissão explícita**
- Teste em laboratório controlado com máquinas dedicadas
- Não use contra sistemas de terceiros sem autorização legal
- Este é um projeto educacional para demonstrar riscos de segurança
- Na defesa do TCC, deixe explícito: ambiente controlado, autorização, metodologia ética

## Customizações

### Mudar para OLED SSD1306
Edite `display_ui.py` e troque o tipo de inicialização (docs no repo Adafruit).

### Adicionar mais botões
Edite `code.py` e adicione mais GPIO (pinos, debounce, etc.).

### Suportar mais payloads
Simplesmente adicione arquivos `.dd` na pasta `payloads/` — o menu lê automaticamente.

## Troubleshooting

**"ImportError: No module named 'adafruit_hid'"**
→ Você não copiou as libs pra `/lib` no Pico. Refaça o passo 2.

**Display não mostra nada**
→ Verifique as ligações I2C (SDA/SCL), endereço do display pode ser diferente (padrão: 0x27).

**Botões não respondem**
→ Revise a ligação dos pinos e pull-down resistors.

**Pico não vira teclado USB**
→ CircuitPython versão antiga. Atualize pro latest (RP2350).

## Projeto Original

Baseado em [dbisu/pico-ducky](https://github.com/dbisu/pico-ducky) (keystroke injection), 
com adições de **display + menu de seleção** pra TCC.

---

**Boa sorte na defesa! 🚀**
