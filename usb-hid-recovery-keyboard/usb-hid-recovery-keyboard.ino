#include "USB.h"
#include "USBHIDKeyboard.h"
#include <Preferences.h>

USBHIDKeyboard Keyboard;
Preferences prefs;
bool soundMuted = false;

const int BUTTON_PIN = 0;
const unsigned long ARM_MS = 1500;
const unsigned long CANCEL_MS = 5000;
const unsigned long MUTE_TOGGLE_MS = 8000;
const unsigned long DEBOUNCE_MS = 40;
const unsigned long ARM_BLINK_MS = 200;
const unsigned long CANCEL_BLINK_MS = 80;
const unsigned long MUTE_BLINK_MS = 120;

const char* WINDOWS_DRIVE = "C:";

const char* SAFEBOOT_ON  = "bcdedit /set {default} safeboot minimal";
const char* SAFEBOOT_OFF = "bcdedit /deletevalue {default} safeboot";
const char* BOOT_TO_RE   = "reagentc /boottore";
const char* RESTART_CMD  = "shutdown /r /t 15 /c \"Reiniciando automaticamente. Rode shutdown /a para cancelar.\"";
const unsigned long DELAY_BETWEEN_COMMANDS_MS = 700;
const unsigned long CHAIN_STEP_DELAY_MS = 2000;

const char* USB_DRIVE         = "E:";
const int   VMD_TARGET_DISK   = 1;
const char  WIN_ASSIGN_LETTER = 'W';
const char* DRIVER_INF_REL    = "IRST_Intel_20.0.0.1038_W11x64\\VMD\\iaStorVD.inf";
const unsigned long DRVLOAD_DELAY_MS   = 3000;
const unsigned long DISKPART_LAUNCH_MS = 6000;

#define USE_RGB_LED 1
const int RGB_PIN = 48;
const uint8_t LED_BRIGHTNESS = 130;

#define USE_BUZZER 1
const int BUZZER_PIN = 4;

enum ActionType { SIMPLE_CMD, AUTO_CHAIN, AUTO_CHAIN2, AUTO_CHAIN3, SAFE_ON, SAFE_OFF, BOOT_RECOVERY, TOGGLE_LAYOUT, DRIVER_INJECT };

struct Mode {
  ActionType type;
  const char* cmd;
  uint8_t r, g, b;
};

const int NUM_MODES = 22;
Mode modes[NUM_MODES];

const int CHAIN1_FIRST = 0,  CHAIN1_LAST = 3;
const int CHAIN2_FIRST = 11, CHAIN2_LAST = 16;
const int CHAIN3_FIRST = 18, CHAIN3_LAST = 19;

const int visibleModeIndices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 17, 20, 21};
const int NUM_VISIBLE_MODES = sizeof(visibleModeIndices) / sizeof(visibleModeIndices[0]);

int currentMode = 0;

char chkdskCmdBuf[32];
char sfcOfflineCmdBuf[96];
char drvloadCmdBuf[160];
char selDiskCmdBuf[32];
char assignCmdBuf[32];
char dismInjectCmdBuf[200];

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
  if (freqHz <= 0 || durationMs <= 0) return;
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
void beepMuteToggle()  { beepRaw(2000, 60); delay(60); beepRaw(2600, 60); }

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
  setLed(255, 128, 0);
  delay(400);
}

void runBootRecoveryAndRestart() {
  Keyboard.println(BOOT_TO_RE);
  flashConfirm();
  delay(DELAY_BETWEEN_COMMANDS_MS);
  Keyboard.println(RESTART_CMD);
  setLed(255, 128, 0);
  delay(400);
}

void toggleKeyboardLayout() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(' ');
  delay(80);
  Keyboard.releaseAll();
}

int promptPartitionNumber() {
  int count = 0;
  while (digitalRead(BUTTON_PIN) == LOW) delay(5);
  delay(DEBOUNCE_MS);

  unsigned long lastBlink = 0;
  bool on = false;
  for (;;) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      unsigned long start = millis();
      while (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long held = millis() - start;
        if (held >= CANCEL_MS) setLed(255, 0, 0);
        else if (held >= ARM_MS) setLed(0, 255, 0);
        delay(5);
      }
      unsigned long held = millis() - start;
      delay(DEBOUNCE_MS);
      if (held >= CANCEL_MS) return -1;
      if (held >= ARM_MS) return count;
      count++;
      beepNavigate();
      setLed(255, 255, 255);
      delay(70);
    } else {
      if (millis() - lastBlink >= 400) {
        lastBlink = millis();
        on = !on;
        setLed(0, on ? 200 : 30, on ? 255 : 60);
      }
      delay(5);
    }
  }
}

bool runDriverInject() {
  Keyboard.println(drvloadCmdBuf);    setLed(0, 0, 255);     delay(DRVLOAD_DELAY_MS);
  Keyboard.println("diskpart");       setLed(150, 0, 255);   delay(DISKPART_LAUNCH_MS);
  Keyboard.println("rescan");         setLed(150, 0, 255);   delay(CHAIN_STEP_DELAY_MS);
  Keyboard.println(selDiskCmdBuf);    setLed(150, 0, 255);   delay(CHAIN_STEP_DELAY_MS);
  Keyboard.println("list partition"); setLed(0, 200, 255);   delay(CHAIN_STEP_DELAY_MS);

  int part = promptPartitionNumber();
  if (part <= 0) {
    Keyboard.println("exit");
    setLed(255, 0, 0);
    beepCancelled();
    delay(400);
    return false;
  }

  char selPartCmdBuf[32];
  snprintf(selPartCmdBuf, sizeof(selPartCmdBuf), "select partition %d", part);
  for (int i = 0; i < part; i++) { beep(2200, 60); delay(120); }

  Keyboard.println(selPartCmdBuf);    setLed(255, 105, 180); delay(CHAIN_STEP_DELAY_MS);
  Keyboard.println(assignCmdBuf);     setLed(0, 255, 120);   delay(CHAIN_STEP_DELAY_MS);
  Keyboard.println("exit");           setLed(0, 255, 120);   delay(DELAY_BETWEEN_COMMANDS_MS);
  Keyboard.println(dismInjectCmdBuf); setLed(0, 255, 0);     delay(400);
  return true;
}

bool executeCurrentMode() {
  const Mode& m = modes[visibleModeIndices[currentMode]];
  switch (m.type) {
    case SIMPLE_CMD:
      Keyboard.println(m.cmd);
      flashConfirm();
      return true;
    case AUTO_CHAIN:
      runChain(CHAIN1_FIRST, CHAIN1_LAST);
      return true;
    case AUTO_CHAIN2:
      runChain(CHAIN2_FIRST, CHAIN2_LAST);
      return true;
    case AUTO_CHAIN3:
      runChain(CHAIN3_FIRST, CHAIN3_LAST);
      return true;
    case SAFE_ON:
      runSafeModeAndRestart(true);
      return true;
    case SAFE_OFF:
      runSafeModeAndRestart(false);
      return true;
    case BOOT_RECOVERY:
      runBootRecoveryAndRestart();
      return true;
    case TOGGLE_LAYOUT:
      toggleKeyboardLayout();
      flashConfirm();
      return true;
    case DRIVER_INJECT:
      return runDriverInject();
  }
  return true;
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

  snprintf(drvloadCmdBuf, sizeof(drvloadCmdBuf), "drvload %s\\%s", USB_DRIVE, DRIVER_INF_REL);
  snprintf(selDiskCmdBuf, sizeof(selDiskCmdBuf), "select disk %d", VMD_TARGET_DISK);
  snprintf(assignCmdBuf, sizeof(assignCmdBuf), "assign letter=%c", WIN_ASSIGN_LETTER);
  snprintf(dismInjectCmdBuf, sizeof(dismInjectCmdBuf),
           "dism /Image:%c:\\ /Add-Driver /Driver:%s\\%s", WIN_ASSIGN_LETTER, USB_DRIVE, DRIVER_INF_REL);

  modes[0] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /CheckHealth",   0,   0, 255};
  modes[1] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /ScanHealth",  255, 255,   0};
  modes[2] = {SIMPLE_CMD, "DISM /Online /Cleanup-Image /RestoreHealth", 255,  0,   0};
  modes[3] = {SIMPLE_CMD, "sfc /scannow",                              255, 255, 255};
  modes[4] = {AUTO_CHAIN, nullptr,                                     255, 110,   0};
  modes[5] = {SIMPLE_CMD, chkdskCmdBuf,                                  0, 255, 120};
  modes[6] = {SIMPLE_CMD, sfcOfflineCmdBuf,                            150,   0, 255};
  modes[7] = {SAFE_ON,    nullptr,                                     255,   0, 180};
  modes[8] = {SAFE_OFF,   nullptr,                                       0, 220, 255};
  modes[9] = {BOOT_RECOVERY, nullptr,                                    0, 255,   0};
  modes[10] = {TOGGLE_LAYOUT, nullptr,                                  170, 255,   0};

  modes[11] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[12] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[13] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[14] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[15] = {SIMPLE_CMD, "e", 75, 0, 255};
  modes[16] = {SIMPLE_CMD, "", 75, 0, 255};
  modes[17] = {AUTO_CHAIN2, nullptr,                                    255, 105, 180};

  modes[18] = {SIMPLE_CMD, "", 255, 140, 90};
  modes[19] = {SIMPLE_CMD, "", 255, 140, 90};
  modes[20] = {AUTO_CHAIN3, nullptr,                                      0, 150, 150};

  modes[21] = {DRIVER_INJECT, nullptr,                                  255, 128, 255};

  USB.begin();
  Keyboard.begin();

  delay(3000);

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
      beepMuteToggle();
    } else if (cancelled) {
    } else if (duration >= ARM_MS) {
      if (executeCurrentMode()) beepExecuted();
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
