"""
Parser e executor de DuckyScript para Pico Ducky
Suporta comandos básicos de HID
"""

import os
import time
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode

class PayloadManager:
    """Gerencia leitura e execução de payloads DuckyScript"""

    # Mapeamento de comandos especiais para keycodes
    SPECIAL_KEYS = {
        "ENTER": Keycode.ENTER,
        "SPACE": Keycode.SPACE,
        "TAB": Keycode.TAB,
        "BACKSPACE": Keycode.BACKSPACE,
        "DELETE": Keycode.DELETE,
        "HOME": Keycode.HOME,
        "END": Keycode.END,
        "ESCAPE": Keycode.ESCAPE,
        "UP": Keycode.UP_ARROW,
        "DOWN": Keycode.DOWN_ARROW,
        "LEFT": Keycode.LEFT_ARROW,
        "RIGHT": Keycode.RIGHT_ARROW,
    }

    MODIFIER_KEYS = {
        "CTRL": Keycode.CONTROL,
        "ALT": Keycode.ALT,
        "SHIFT": Keycode.SHIFT,
        "GUI": Keycode.GUI,
        "WINDOWS": Keycode.GUI,
    }

    def __init__(self, keyboard=None, layout=None):
        """
        Inicializa o gerenciador de payloads

        Args:
            keyboard: instância Adafruit Keyboard (criar com usb_hid.devices.get_device...)
            layout: layout de teclado (padrão: US)
        """
        self.keyboard = keyboard
        self.layout = layout or KeyboardLayoutUS()
        self.payloads_dir = "/payloads"

    def list_payloads(self):
        """Lista todos os payloads .dd na pasta /payloads"""
        try:
            files = os.listdir(self.payloads_dir)
            # Filtra apenas .dd, ordena
            payloads = sorted([f for f in files if f.endswith(".dd")])
            return payloads
        except OSError:
            print(f"[AVISO] Pasta {self.payloads_dir} não encontrada")
            return []

    def load_payload(self, filename):
        """
        Carrega um payload .dd e retorna linhas (sem comentários)

        Args:
            filename: nome do arquivo na pasta /payloads

        Returns:
            Lista de strings (comandos)
        """
        path = f"{self.payloads_dir}/{filename}"
        try:
            with open(path, "r") as f:
                lines = f.readlines()
            # Remove comentários (#) e linhas vazias
            commands = []
            for line in lines:
                line = line.strip()
                if line and not line.startswith("#"):
                    commands.append(line)
            return commands
        except OSError as e:
            print(f"[ERRO] Não consegui ler {path}: {e}")
            return []

    def execute_payload(self, filename):
        """
        Executa um payload .dd

        Args:
            filename: nome do arquivo na pasta /payloads

        Returns:
            True se sucesso, False se erro
        """
        if not self.keyboard:
            print("[ERRO] Keyboard não inicializado")
            return False

        commands = self.load_payload(filename)
        if not commands:
            print(f"[ERRO] Payload vazio ou não encontrado: {filename}")
            return False

        print(f"[EXEC] Executando: {filename}")

        try:
            for cmd in commands:
                self._execute_command(cmd)
            print(f"[OK] {filename} concluído")
            return True
        except Exception as e:
            print(f"[ERRO] Ao executar {filename}: {e}")
            return False

    def _execute_command(self, cmd):
        """
        Executa um comando individual

        Formatos suportados:
            DELAY <ms>              → aguarda
            STRING <texto>          → digita texto
            ENTER                   → pressiona Enter
            SPACE                   → espaço
            TAB                     → tabulação
            GUI r                   → Windows+R
            CTRL a                  → Ctrl+A
            etc.
        """
        parts = cmd.split(maxsplit=1)
        command = parts[0].upper()
        arg = parts[1] if len(parts) > 1 else ""

        if command == "DELAY":
            try:
                ms = int(arg)
                time.sleep(ms / 1000.0)
            except ValueError:
                print(f"[AVISO] DELAY inválido: {arg}")

        elif command == "STRING":
            # Digita o texto usando layout
            self.keyboard.write(arg, self.layout)

        elif command in self.SPECIAL_KEYS:
            # Tecla especial simples (ENTER, SPACE, etc)
            self.keyboard.press(self.SPECIAL_KEYS[command])
            self.keyboard.release(self.SPECIAL_KEYS[command])

        elif command in self.MODIFIER_KEYS:
            # Comando com modificador (CTRL a, GUI r, ALT TAB)
            keys = arg.split()
            if keys:
                modifier = self.MODIFIER_KEYS[command]
                self.keyboard.press(modifier)

                # Se houver segundo argumento, pressiona a tecla
                for key in keys:
                    key_code = self._get_keycode(key)
                    if key_code:
                        self.keyboard.press(key_code)
                        self.keyboard.release(key_code)

                self.keyboard.release(modifier)

    def _get_keycode(self, key_name):
        """
        Converte nome da tecla em keycode

        Args:
            key_name: nome como 'a', 'r', 'TAB', 'ENTER'

        Returns:
            Keycode ou None
        """
        key_upper = key_name.upper()

        # Teclas especiais
        if key_upper in self.SPECIAL_KEYS:
            return self.SPECIAL_KEYS[key_upper]

        # Letra única (a-z)
        if len(key_name) == 1 and key_name.isalpha():
            # Converte pra keycode (a=0x04, b=0x05, etc)
            return Keycode.A + (ord(key_upper) - ord('A'))

        # Número (0-9)
        if len(key_name) == 1 and key_name.isdigit():
            return Keycode.ZERO + int(key_name)

        # Símbolos comuns
        symbols = {
            ".": Keycode.PERIOD,
            ",": Keycode.COMMA,
            "!": Keycode.EXCLAMATION,
            "@": Keycode.AT,
            "#": Keycode.POUND,
            "$": Keycode.DOLLAR,
            "%": Keycode.PERCENT,
            "^": Keycode.CARET,
            "&": Keycode.AMPERSAND,
            "*": Keycode.ASTERISK,
            "(": Keycode.LEFT_PAREN,
            ")": Keycode.RIGHT_PAREN,
            "-": Keycode.MINUS,
            "_": Keycode.UNDERSCORE,
            "=": Keycode.EQUALS,
            "+": Keycode.PLUS,
            "[": Keycode.LEFT_BRACKET,
            "]": Keycode.RIGHT_BRACKET,
            "{": Keycode.LEFT_BRACE,
            "}": Keycode.RIGHT_BRACE,
            ";": Keycode.SEMICOLON,
            ":": Keycode.COLON,
            "'": Keycode.QUOTE,
            '"': Keycode.QUOTE,
            "/": Keycode.FORWARD_SLASH,
            "\\": Keycode.BACKSLASH,
            "<": Keycode.LESS_THAN,
            ">": Keycode.GREATER_THAN,
            "?": Keycode.QUESTION,
            "~": Keycode.TILDE,
            "`": Keycode.BACKTICK,
        }

        if key_name in symbols:
            return symbols[key_name]

        return None
