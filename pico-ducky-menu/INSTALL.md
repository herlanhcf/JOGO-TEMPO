# Guia de Instalação - Pico Ducky Menu

Passo-a-passo completo pra deixar seu Pico 2 pronto com firmware e libs.

## Passo 1: Preparar o CircuitPython

### 1.1 Baixar CircuitPython RP2350

Acesse: https://circuitpython.org/board/raspberry_pi_pico2/

Clique em **"Download UF2"** e guarde o arquivo `.uf2` (ex: `adafruit-circuitpython-raspberry_pi_pico2-9.x.x.uf2`).

### 1.2 Flashear no Pico 2

1. Segure o botão **BOOTSEL** do seu Pico 2 (durante 5 segundos)
2. Plugue o Pico via USB-C no computador **enquanto segura BOOTSEL**
3. Uma unidade chamada **RPI-RP2** deve aparecer no explorador de arquivos
4. **Arraste o arquivo .uf2** pra dentro dessa unidade
5. O Pico vai reiniciar e aparecer uma unidade chamada **CIRCUITPY**

Pronto! CircuitPython está rodando.

## Passo 2: Instalar Bibliotecas Adafruit

Você precisa baixar 3 bibliotecas e copiar pra pasta `/lib` do seu Pico.

### 2.1 Baixar as Libs

**Adafruit HID:**
- GitHub: https://github.com/adafruit/Adafruit_CircuitPython_HID
- Clique: Code → Download ZIP
- Deszip e procure a pasta `adafruit_hid` (não o ZIP inteiro)

**Adafruit Bus Device:**
- GitHub: https://github.com/adafruit/Adafruit_CircuitPython_BusDevice
- Mesmo processo: Code → Download ZIP
- Procure por `adafruit_bus_device`

**Adafruit LCD Character Display:**
- GitHub: https://github.com/adafruit/Adafruit_CircuitPython_CharLCD
- Mesmo: Code → Download ZIP
- Procure por `adafruit_character_lcd`

### 2.2 Copiar Libs pro Pico

1. Abra a unidade **CIRCUITPY** (seu Pico aparece como unidade USB)
2. Crie uma pasta chamada **`lib`** (se não existir)
3. Dentro de `lib/`, copie as 3 pastas:
   ```
   CIRCUITPY/
   ├── lib/
   │   ├── adafruit_hid/
   │   ├── adafruit_bus_device/
   │   └── adafruit_character_lcd/
   ├── code.py (seu firmware)
   └── boot.py (do CircuitPython)
   ```

**Teste rápido:** No REPL (Ctrl+C na serial, depois Ctrl+C novamente), digita:
```python
>>> import adafruit_hid
>>> print("OK")
```

Se "OK" aparecer, as libs estão corretas.

## Passo 3: Instalar Firmware do Pico Ducky

### 3.1 Clonar ou Baixar este Repo

Você já tem essa pasta em `/home/user/JOGO-TEMPO/pico-ducky-menu/`.

### 3.2 Copiar Arquivos pro Pico

Copie estes arquivos pra raiz da unidade **CIRCUITPY**:

- `code.py` → **CIRCUITPY/code.py**
- `payloads.py` → **CIRCUITPY/payloads.py**
- `display_ui.py` → **CIRCUITPY/display_ui.py**

### 3.3 Criar Pasta de Payloads

1. Na unidade **CIRCUITPY**, crie uma pasta chamada **`payloads`**
2. Copie os arquivos `.dd`:
   ```
   CIRCUITPY/
   ├── payloads/
   │   ├── demo.dd
   │   ├── run_cmd.dd
   │   └── info_stealer.dd
   ```

Pronto! O Pico vai detectar os payloads automaticamente.

## Passo 4: Testar Hardware

Antes de usar em produção, test tudo:

### 4.1 Display I2C

Descomente e rode no REPL:
```python
import busio
import board

i2c = busio.I2C(board.GP5, board.GP4)
print([hex(x) for x in i2c.scan()])
```

Deve aparecer o endereço do display (típico: `0x27` ou `0x3F`).

### 4.2 Botões

Toque em cada botão e veja no REPL (serial):
```python
import digitalio, board
btn = digitalio.DigitalInOut(board.GP10)
btn.direction = digitalio.Direction.INPUT
while True:
    print(btn.value)  # 1 = pressionado, 0 = solto
```

### 4.3 Teclado USB HID

Quando o Pico está rodando `code.py`, vá pra um editor de texto e pressione o botão **SELECT**. Deve começar a digitar.

## Passo 5: Customizar Payloads

Crie seus próprios payloads. Exemplo `meu_payload.dd`:

```
# Meu payload customizado
DELAY 1000
GUI r
DELAY 300
STRING calc
ENTER
```

Copie pra `CIRCUITPY/payloads/meu_payload.dd` e já aparece no menu!

## Troubleshooting

### "No module named 'adafruit_hid'"

**Solução:**
1. Revise se copiou a pasta `adafruit_hid/` (completa, não só o arquivo `.py`)
2. Tente usar o REPL: `import adafruit_hid` (deve funcionar)
3. Se continuar, re-download a lib e copie novamente

### Display mostra "ERRO" ou não liga

**Causas comuns:**
1. Pinos I2C (GP4/GP5) soltos ou com mau contato
2. Endereço I2C errado (edite `code.py`, linha ~20: `address=0x3F`)
3. Display sem alimentação (verifique 5V/GND)

**Teste:**
```python
# No REPL, varre endereços I2C:
import busio, board
i2c = busio.I2C(board.GP5, board.GP4)
print(i2c.scan())  # deve mostrar [0x27] ou similar
```

### Botões não respondem

1. Revise resistores 10kΩ (pull-down)
2. Teste no REPL:
   ```python
   import digitalio, board, time
   btn = digitalio.DigitalInOut(board.GP10)
   btn.direction = digitalio.Direction.INPUT
   for _ in range(100):
       print(btn.value)
       time.sleep(0.05)
   ```
3. Pressione e veja valor mudar de 0 pra 1

### Pico não funciona como teclado

1. CircuitPython versão antiga. Atualize pro latest RP2350.
2. Teste num computador diferente
3. Use cabo USB-C de qualidade (alguns cabos são "charge only")

### REPL não aparece

1. Instale o [CircuitPython IDE](https://circuitpython.org/board/raspberry_pi_pico2/) ou use **Thonny**
2. Conecte via porta COM/serial

---

## Checklist Final

- [ ] CircuitPython RP2350 instalado
- [ ] Libs Adafruit em `/lib`
- [ ] `code.py`, `payloads.py`, `display_ui.py` copiados
- [ ] Pasta `/payloads` com `.dd` files
- [ ] Display aparece no I2C scan
- [ ] Botões respondem no REPL
- [ ] Teclado funciona (toque no botão SELECT, vê digitação num editor)

**Pronto pra apresentação!** 🚀
