#include "USB.h"
#include "USBHIDKeyboard.h"
#include <Preferences.h>

USBHIDKeyboard Keyboard;
Preferences prefs;
bool soundMuted = false; // carregado da memoria (NVS) no setup()

const int BUTTON_PIN = 0;          // GPIO0 = botao BOOT da placa, ativo em LOW
const unsigned long ARM_MS = 1500;         // tempo segurando ate armar (piscar) e poder executar ao soltar
const unsigned long CANCEL_MS = 5000;      // tempo segurando ate cancelar (piscar vermelho)
const unsigned long MUTE_TOGGLE_MS = 8000; // tempo segurando ate a faixa de ligar/desligar o som
const unsigned long DEBOUNCE_MS = 40;
const unsigned long ARM_BLINK_MS = 200;
const unsigned long CANCEL_BLINK_MS = 80;
const unsigned long MUTE_BLINK_MS = 120;

// Se dentro do WinRE o Windows nao estiver em C:, troque aqui antes de gravar.
const char* WINDOWS_DRIVE = "C:";

const char* SAFEBOOT_ON  = "bcdedit /set {default} safeboot minimal";
const char* SAFEBOOT_OFF = "bcdedit /deletevalue {default} safeboot";
const char* BOOT_TO_RE   = "reagentc /boottore";
const char* RESTART_CMD  = "shutdown /r /t 15 /c \"Reiniciando automaticamente. Rode shutdown /a para cancelar.\"";
const unsigned long DELAY_BETWEEN_COMMANDS_MS = 700; // da tempo do comando anterior terminar
const unsigned long CHAIN_STEP_DELAY_MS = 2000;      // espacamento entre comandos dos modos auto-chain

#define USE_RGB_LED 1
const int RGB_PIN = 48;
const uint8_t LED_BRIGHTNESS = 130; // 0-255, aplicado em cima da cor "pura" de cada modo

#define USE_BUZZER 1
const int BUZZER_PIN = 4;

enum ActionType { SIMPLE_CMD, AUTO_CHAIN, AUTO_CHAIN2, AUTO_CHAIN3, SAFE_ON, SAFE_OFF, BOOT_RECOVERY, TOGGLE_LAYOUT };

struct Mode {
  ActionType type;
  const char* cmd; // usado so quando type == SIMPLE_CMD
  uint8_t r, g, b;
};

const int NUM_MODES = 21; // tamanho do array modes[] (inclui os passos internos das correntes)
Mode modes[NUM_MODES];

// Limites (inclusivos) dos passos internos de cada corrente automatica. Deixar
// isso em constantes -- em vez de numeros soltos espalhados pelo codigo --
// significa que adicionar/remover um passo de uma corrente e uma edicao so:
// mexe aqui e no bloco modes[] correspondente, e os loops se ajustam sozinhos.
const int CHAIN1_FIRST = 0,  CHAIN1_LAST = 3;   // AUTO_CHAIN (indice 4) replica os modos visiveis 0..3
const int CHAIN2_FIRST = 11, CHAIN2_LAST = 16;  // passos internos da AUTO_CHAIN2 (indice 17)
const int CHAIN3_FIRST = 18, CHAIN3_LAST = 19;  // passos internos da AUTO_CHAIN3 (indice 20)

// Só estes indices aparecem na navegacao (clique curto) e no LED. Os passos
// internos das correntes automaticas (11-16 usados pela AUTO_CHAIN2 e 18-19
// usados pela AUTO_CHAIN3) ficam de fora daqui -- nunca aparecem como modo
// separado ao navegar, so sao visitados (e digitados) quando a corrente que
// os contem e executada.
const int visibleModeIndices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 17, 20};
const int NUM_VISIBLE_MODES = sizeof(visibleModeIndices) / sizeof(visibleModeIndices[0]);

int currentMode = 0; // indice DENTRO de visibleModeIndices, nao direto em modes[]

char chkdskCmdBuf[32];
char sfcOfflineCmdBuf[96];

bool pressed = false;
unsigned long pressStart = 0;
bool armed = false;
bool cancelled = false;
bool muteZone = false;
bool armedBeeped = false;
bool cancelledBeeped = false;
unsigned long lastBlinkToggle = 0;
bool blinkOn = false;

void beepRaw(int freqHz, int durationMs) {
#if USE_BUZZER
  if (freqHz <= 0 || durationMs <= 0) return; // evita divisao por zero / laco negativo
  long halfPeriodUs = 500000L / freqHz;
  long cycles = (long)durationMs * 1000L / (halfPeriodUs * 2);
  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(halfPeriodUs);
  }
#endif
}

void beep(int freqHz, int durationMs) {
  if (soundMuted) return;
  beepRaw(freqHz, durationMs);
}

void beepStartup()    { beep(1800, 90); }
void beepNavigate()   { beep(1500, 18); }
void beepArmed()       { beep(2400, 70); }
void beepExecuted()    { beep(2200, 55); delay(50); beep(2200, 55); }
void beepCancelled()   { beep(350, 260); }
void beepMuteToggle()  { beepRaw(2000, 60); delay(60); beepRaw(2600, 60); } // sempre audivel

// Ponto unico de escrita no LED -- aplica LED_BRIGHTNESS e o #if USE_RGB_LED,
// entao o resto do codigo so chama setLed(r,g,b) sem se preocupar com nenhum
// dos dois.
void setLed(uint8_t r, uint8_t g, uint8_t b) {
#if USE_RGB_LED
  neopixelWrite(RGB_PIN,
                (uint16_t)r * LED_BRIGHTNESS / 255,
                (uint16_t)g * LED_BRIGHTNESS / 255,
                (uint16_t)b * LED_BRIGHTNESS / 255);
#endif
}

void showModeColor() {
  const Mode& m = modes[visibleModeIndices[currentMode]];
  setLed(m.r, m.g, m.b);
}

void flashConfirm() {
  setLed(0, 0, 0);
  delay(120);
  showModeColor();
}

// Digita, em sequencia, os comandos de modes[first..last] (inclusivo),
// acendendo a cor de cada passo. Usado pelas tres correntes automaticas.
void runChain(int first, int last) {
  for (int i = first; i <= last; i++) {
    Keyboard.println(modes[i].cmd);
    setLed(modes[i].r, modes[i].g, modes[i].b);
    delay(CHAIN_STEP_DELAY_MS);
  }
}

void runSafeModeAndRestart(bool turnOn) {
  Keyboard.println(turnOn ? SAFEBOOT_ON : SAFEBOOT_OFF);
  flashConfirm();
  delay(DELAY_BETWEEN_COMMANDS_MS);
  Keyboard.println(RESTART_CMD);
  setLed(255, 128, 0); // laranja = reinicio agendado (15s pra cancelar)
  delay(400);
}

void runBootRecoveryAndRestart() {
  Keyboard.println(BOOT_TO_RE);
  flashConfirm();
  delay(DELAY_BETWEEN_COMMANDS_MS);
  Keyboard.println(RESTART_CMD);
  setLed(255, 128, 0); // laranja = reinicio agendado (15s pra cancelar)
  delay(400);
}

void toggleKeyboardLayout() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(' ');
  delay(80);
  Keyboard.releaseAll();
}

void executeCurrentMode() {
  const Mode& m = modes[visibleModeIndices[currentMode]];
  switch (m.type) {
    case SIMPLE_CMD:
      Keyboard.println(m.cmd);
      flashConfirm();
      break;
    case AUTO_CHAIN:
      runChain(CHAIN1_FIRST, CHAIN1_LAST);
      break;
    case AUTO_CHAIN2:
      runChain(CHAIN2_FIRST, CHAIN2_LAST);
      break;
    case AUTO_CHAIN3:
      runChain(CHAIN3_FIRST, CHAIN3_LAST);
      break;
    case SAFE_ON:
      runSafeModeAndRestart(true);
      break;
    case SAFE_OFF:
      runSafeModeAndRestart(false);
      break;
    case BOOT_RECOVERY:
      runBootRecoveryAndRestart();
      break;
    case TOGGLE_LAYOUT:
      toggleKeyboardLayout();
      flashConfirm();
      break;
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  prefs.begin("esp32btn", false);
  soundMuted = prefs.getBool("muted", false);

  snprintf(chkdskCmdBuf, sizeof(chkdskCmdBuf), "chkdsk %s /f", WINDOWS_DRIVE);
  snprintf(sfcOfflineCmdBuf, sizeof(sfcOfflineCmdBuf),
           "sfc /scannow /offbootdir=%s\\ /offwindir=%s\\Windows", WINDOWS_DRIVE, WINDOWS_DRIVE);

  modes[0] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /CheckHealth",   0,   0, 255}; // azul
  modes[1] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /ScanHealth",  255, 255,   0}; // amarelo
  modes[2] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /RestoreHealth", 255,  0,   0}; // vermelho
  modes[3] = {SIMPLE_CMD, "sfc /scannow",                              255, 255, 255}; // branco
  modes[4] = {AUTO_CHAIN, nullptr,                                     255, 110,   0}; // laranja
  modes[5] = {SIMPLE_CMD, chkdskCmdBuf,                                  0, 255, 120}; // verde-agua
  modes[6] = {SIMPLE_CMD, sfcOfflineCmdBuf,                            150,   0, 255}; // roxo
  modes[7] = {SAFE_ON,    nullptr,                                     255,   0, 180}; // magenta
  modes[8] = {SAFE_OFF,   nullptr,                                       0, 220, 255}; // ciano
  modes[9] = {BOOT_RECOVERY, nullptr,                                    0, 255,   0}; // verde
  modes[10] = {TOGGLE_LAYOUT, nullptr,                                  170, 255,   0}; // verde-limao

  // Passos internos da corrente AUTO_CHAIN2 (indice 17). Nao aparecem
  // sozinhos na navegacao -- ver visibleModeIndices acima. Texto placeholder;
  // troque pelos comandos reais antes de gravar de verdade. O intervalo esta
  // em CHAIN2_FIRST..CHAIN2_LAST, entao basta acrescentar/remover linhas aqui
  // e ajustar essas duas constantes -- nada de numeros soltos noutro lugar.
  modes[11] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[12] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[13] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[14] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[15] = {SIMPLE_CMD, "e", 75, 0, 255};
  modes[16] = {SIMPLE_CMD, "", 75, 0, 255}; // linha extra da corrente (placeholder: troque pelo comando real)
  modes[17] = {AUTO_CHAIN2, nullptr,                                    255, 105, 180}; // rosa-choque

  // Passos internos da corrente AUTO_CHAIN3 (indice 20). Mesma logica: so
  // sao digitados quando a corrente roda, nunca aparecem sozinhos.
  modes[18] = {SIMPLE_CMD, "", 255, 140, 90};
  modes[19] = {SIMPLE_CMD, "", 255, 140, 90};
  modes[20] = {AUTO_CHAIN3, nullptr,                                      0, 150, 150}; // azul-petroleo

  USB.begin();
  Keyboard.begin();

  delay(3000); // da tempo do Windows reconhecer o dispositivo HID antes do primeiro uso

  showModeColor();
  beepStartup();
}

void loop() {
  bool btnDown = digitalRead(BUTTON_PIN) == LOW;

  if (btnDown && !pressed) {
    pressed = true;
    pressStart = millis();
    armed = false;
    cancelled = false;
    muteZone = false;
    armedBeeped = false;
    cancelledBeeped = false;
  }

  if (pressed && btnDown) {
    unsigned long held = millis() - pressStart;

    if (held >= MUTE_TOGGLE_MS) {
      muteZone = true;
      if (millis() - lastBlinkToggle >= MUTE_BLINK_MS) {
        lastBlinkToggle = millis();
        blinkOn = !blinkOn;
        if (blinkOn) setLed(255, 255, 255);
        else setLed(0, 0, 0);
      }
    } else if (held >= CANCEL_MS) {
      cancelled = true;
      if (!cancelledBeeped) {
        cancelledBeeped = true;
        beepCancelled();
      }
      if (millis() - lastBlinkToggle >= CANCEL_BLINK_MS) {
        lastBlinkToggle = millis();
        blinkOn = !blinkOn;
        if (blinkOn) setLed(255, 0, 0);
        else setLed(0, 0, 0);
      }
    } else if (held >= ARM_MS) {
      armed = true;
      if (!armedBeeped) {
        armedBeeped = true;
        beepArmed();
      }
      if (millis() - lastBlinkToggle >= ARM_BLINK_MS) {
        lastBlinkToggle = millis();
        blinkOn = !blinkOn;
        if (blinkOn) {
          const Mode& m = modes[visibleModeIndices[currentMode]];
          setLed(m.r, m.g, m.b);
        } else setLed(0, 0, 0);
      }
    }
  }

  if (!btnDown && pressed) {
    pressed = false;
    unsigned long duration = millis() - pressStart;
    delay(DEBOUNCE_MS);

    if (muteZone) {
      soundMuted = !soundMuted;
      prefs.putBool("muted", soundMuted);
      beepMuteToggle(); // sempre audivel, mesmo que va desligar o som agora
    } else if (cancelled) {
      // solto entre 5s e 8s: nao faz nada (ja beepou o aviso)
    } else if (duration >= ARM_MS) {
      executeCurrentMode();
      beepExecuted();
    } else {
      currentMode = (currentMode + 1) % NUM_VISIBLE_MODES;
      beepNavigate();
    }

    armed = false;
    cancelled = false;
    muteZone = false;
    showModeColor();
  }

  delay(5);
}
