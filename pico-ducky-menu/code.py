"""
Pico Ducky Menu - Firmware Principal
Suporta HID keyboard + display LCD + 3 botões pra seleção de payloads
"""

import board
import digitalio
import time
import usb_hid

from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS

from display_ui import DisplayUI
from payloads import PayloadManager

# Configuração de pinos
PIN_BTN_UP = board.GP10      # Botão UP
PIN_BTN_SELECT = board.GP11  # Botão SELECT
PIN_BTN_DOWN = board.GP12    # Botão DOWN

# Debounce em ms
DEBOUNCE_MS = 50

class DuckyMenu:
    """Menu principal do Pico Ducky"""

    def __init__(self):
        """Inicializa o firmware"""
        print("[BOOT] Pico Ducky Menu iniciando...")

        # Display
        self.display = DisplayUI(sda_pin=board.GP4, scl_pin=board.GP5)

        # Keyboard USB HID
        self.keyboard = Keyboard(usb_hid.devices)
        self.layout = KeyboardLayoutUS()

        # Payload Manager
        self.payloads_mgr = PayloadManager(self.keyboard, self.layout)

        # Botões
        self.btn_up = self._init_button(PIN_BTN_UP, "UP")
        self.btn_select = self._init_button(PIN_BTN_SELECT, "SELECT")
        self.btn_down = self._init_button(PIN_BTN_DOWN, "DOWN")

        # Estado do menu
        self.payloads = self.payloads_mgr.list_payloads()
        self.selected_idx = 0
        self.last_btn_time = 0

        print(f"[BOOT] {len(self.payloads)} payload(s) encontrado(s)")
        self._show_menu()

    def _init_button(self, pin, name):
        """Inicializa um botão com pull-down"""
        btn = digitalio.DigitalInOut(pin)
        btn.direction = digitalio.Direction.INPUT
        btn.pull = digitalio.Pull.DOWN
        print(f"[BOOT] Botão {name} inicializado em {pin}")
        return btn

    def _show_menu(self):
        """Atualiza a exibição do menu no display"""
        if self.payloads:
            payload_name = self.payloads[self.selected_idx]
            display_name = payload_name.replace(".dd", "")
            self.display.show_menu(
                [p.replace(".dd", "") for p in self.payloads],
                self.selected_idx
            )
            print(f"[MENU] Seleção: {display_name} ({self.selected_idx + 1}/{len(self.payloads)})")
        else:
            self.display.show_error("Nenhum payload")
            print("[ERRO] Nenhum payload .dd encontrado em /payloads")

    def _on_btn_up(self):
        """Botão UP pressionado"""
        if self.payloads and self.selected_idx > 0:
            self.selected_idx -= 1
            self._show_menu()

    def _on_btn_down(self):
        """Botão DOWN pressionado"""
        if self.payloads and self.selected_idx < len(self.payloads) - 1:
            self.selected_idx += 1
            self._show_menu()

    def _on_btn_select(self):
        """Botão SELECT pressionado - executa payload"""
        if not self.payloads:
            self.display.show_error("Nenhum payload")
            return

        payload = self.payloads[self.selected_idx]
        print(f"[EXEC] Executando: {payload}")

        payload_name = payload.replace(".dd", "")
        self.display.show_running(payload_name)

        # Pequeno delay antes de executar (safety)
        time.sleep(1)

        # Executa o payload
        success = self.payloads_mgr.execute_payload(payload)

        if success:
            self.display.show_done(payload_name)
            time.sleep(2)  # Mostra mensagem de conclusão por 2s
        else:
            self.display.show_error("Erro ao executar")
            time.sleep(2)

        # Volta ao menu
        self._show_menu()

    def _debounce_check(self):
        """Verifica se é seguro processar input (debounce)"""
        now = time.monotonic() * 1000  # ms
        if now - self.last_btn_time > DEBOUNCE_MS:
            self.last_btn_time = now
            return True
        return False

    def run(self):
        """Loop principal - aguarda botões"""
        print("[RUN] Aguardando entrada...")

        while True:
            # Lê botões
            if self.btn_up.value and self._debounce_check():
                print("[BTN] UP pressionado")
                self._on_btn_up()
                time.sleep(0.1)

            if self.btn_down.value and self._debounce_check():
                print("[BTN] DOWN pressionado")
                self._on_btn_down()
                time.sleep(0.1)

            if self.btn_select.value and self._debounce_check():
                print("[BTN] SELECT pressionado")
                self._on_btn_select()
                time.sleep(0.1)

            time.sleep(0.05)  # Pequeno delay pra não sobrecarregar


# ============================================================================
# MAIN
# ============================================================================

if __name__ == "__main__":
    try:
        menu = DuckyMenu()
        menu.run()
    except KeyboardInterrupt:
        print("\n[STOP] Interrupção por usuário")
    except Exception as e:
        print(f"\n[FATAL] Erro: {e}")
        import traceback
        traceback.print_exc()
