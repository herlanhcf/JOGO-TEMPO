"""
Controle de display LCD 16x2 I2C para Pico Ducky Menu
Suporta displays LCD 16x2 com driver PCF8574
"""

import busio
import board
from adafruit_character_lcd.character_lcd_i2c import Character_LCD_I2C

class DisplayUI:
    """Interface de display LCD via I2C"""

    def __init__(self, sda_pin=board.GP4, scl_pin=board.GP5, address=0x27, cols=16, rows=2):
        """
        Inicializa o display LCD via I2C

        Args:
            sda_pin: pino SDA (padrão GPIO 4)
            scl_pin: pino SCL (padrão GPIO 5)
            address: endereço I2C do driver PCF8574 (padrão 0x27, pode ser 0x3F)
            cols: número de colunas (16 ou 20)
            rows: número de linhas (2 ou 4)
        """
        try:
            self.i2c = busio.I2C(scl_pin, sda_pin)
            self.lcd = Character_LCD_I2C(self.i2c, cols, rows, address)
            self.lcd.clear()
            self.cols = cols
            self.rows = rows
            self.is_ready = True
            self.show_boot_message()
        except Exception as e:
            print(f"[ERRO] Display não encontrado: {e}")
            self.is_ready = False

    def show_boot_message(self):
        """Mensagem de boot do firmware"""
        self.clear()
        self.print_line(0, "Pico Ducky Menu")
        self.print_line(1, "Carregando...")

    def clear(self):
        """Limpa o display"""
        if self.is_ready:
            try:
                self.lcd.clear()
            except:
                pass

    def print_line(self, line_num, text):
        """Escreve texto em uma linha específica (0 ou 1 para 16x2)"""
        if not self.is_ready:
            return

        try:
            # Limita texto ao número de colunas
            text = str(text)[:self.cols]
            # Padding com espaços pra limpar restos de texto anterior
            text = text.ljust(self.cols)
            self.lcd.message = text if line_num == 0 else self.lcd.message.split('\n')[0] + '\n' + text
        except:
            pass

    def show_menu(self, payloads, selected_idx):
        """Mostra o menu de seleção de payloads"""
        if not self.is_ready:
            return

        self.clear()

        # Linha 0: payload anterior (ou vazio)
        if selected_idx > 0:
            prev_name = payloads[selected_idx - 1][:self.cols]
            self.print_line(0, f"> {prev_name}")
        else:
            self.print_line(0, "  ---------")

        # Linha 1: payload selecionado (highlight)
        if selected_idx < len(payloads):
            curr_name = payloads[selected_idx][:self.cols - 2]
            self.lcd.cursor = True
            self.lcd.blink = True
            self.print_line(1, f"* {curr_name}")

        # TODO: Exibir também payload seguinte num display de 4 linhas

    def show_running(self, payload_name):
        """Indica que um payload está sendo executado"""
        if not self.is_ready:
            return

        self.clear()
        self.print_line(0, "Executando:")
        name = payload_name[:self.cols]
        self.print_line(1, name)

    def show_done(self, payload_name):
        """Indica conclusão"""
        if not self.is_ready:
            return

        self.clear()
        self.print_line(0, "Concluido!")
        name = payload_name[:self.cols]
        self.print_line(1, name)

    def show_error(self, error_msg):
        """Mostra mensagem de erro"""
        if not self.is_ready:
            return

        self.clear()
        self.print_line(0, "ERRO")
        msg = str(error_msg)[:self.cols]
        self.print_line(1, msg)

    def show_status(self, line0, line1):
        """Mostra duas linhas de status customizadas"""
        if not self.is_ready:
            return

        self.clear()
        self.print_line(0, str(line0)[:self.cols])
        self.print_line(1, str(line1)[:self.cols])
