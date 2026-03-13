#include <Arduino.h>
#include <Wire.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Encoder.h>

#if __has_include(<HID-Project.h>)
#include <HID-Project.h>
#define HAS_HID_PROJECT 1
#else
#define HAS_HID_PROJECT 0
#endif

// Pin map (RP2040 GP numbers)
const uint8_t ROW_PINS[3] = {2, 3, 4};
const uint8_t COL_PINS[3] = {5, 6, 7};
const uint8_t ENC_A_PIN = 8;
const uint8_t ENC_B_PIN = 9;
const uint8_t ENC_BTN_PIN = 10;
const uint8_t RGB_PIN = 15;

const uint8_t LED_COUNT = 9;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel pixels(LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);
Encoder encoder(ENC_A_PIN, ENC_B_PIN);

enum Layer {
  L_BASE = 0,
  L_EDIT = 1,
  L_MEDIA = 2,
  L_FN = 3,
  L_RGB = 4,
  L_SELECT = 5,
  L_SYS = 6,
  L_NAV = 7,
  L_NUM = 8,
  L_COUNT = 9
};

const char *LAYER_NAMES[L_COUNT] = {
  "BASE", "EDIT", "MEDIA", "FN", "RGB", "SELECT", "SYS", "NAV", "NUM"
};

volatile uint8_t activeLayer = L_BASE;
volatile uint8_t pendingLayer = L_BASE;

bool keyDown[9] = {false};
bool btnPressed = false;
bool btnTurned = false;
bool rgbOn = true;
uint8_t rgbHue = 150;
uint8_t rgbSat = 255;
uint8_t rgbVal = 80;
int16_t rgbSpeed = 8;

long lastEncoderPos = 0;
unsigned long lastDisplayMs = 0;

uint32_t layerColor(uint8_t layer) {
  switch (layer) {
    case L_BASE: return pixels.ColorHSV(149UL * 256, 255, rgbVal);
    case L_EDIT: return pixels.ColorHSV(64UL * 256, 255, rgbVal);
    case L_MEDIA: return pixels.ColorHSV(170UL * 256, 255, rgbVal);
    case L_FN: return pixels.ColorHSV(213UL * 256, 255, rgbVal);
    case L_RGB: return pixels.ColorHSV(0UL * 256, 255, rgbVal);
    case L_SELECT: return pixels.ColorHSV(85UL * 256, 255, rgbVal);
    case L_SYS: return pixels.ColorHSV(30UL * 256, 255, rgbVal);
    case L_NAV: return pixels.ColorHSV(110UL * 256, 255, rgbVal);
    case L_NUM: return pixels.ColorHSV(190UL * 256, 255, rgbVal);
    default: return pixels.ColorHSV(149UL * 256, 255, rgbVal);
  }
}

void applyRgb() {
  if (!rgbOn) {
    pixels.clear();
    pixels.show();
    return;
  }

  uint32_t c = layerColor(activeLayer);
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pixels.setPixelColor(i, c);
  }
  pixels.show();
}

void drawStatus() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Silent 3x3 (Arduino)");
  display.print("Layer: ");
  display.println(LAYER_NAMES[activeLayer]);

  if (activeLayer == L_SELECT) {
    display.print("Goto : ");
    display.println(LAYER_NAMES[pendingLayer]);
  }

  display.print("RGB  : ");
  display.println(rgbOn ? "ON" : "OFF");
  display.print("Val  : ");
  display.println((int)rgbVal);
  display.display();
}

void tapKey(uint8_t k) {
  Keyboard.press(k);
  delay(2);
  Keyboard.release(k);
}

void tapCombo2(uint8_t mod, uint8_t key) {
  Keyboard.press(mod);
  Keyboard.press(key);
  delay(5);
  Keyboard.releaseAll();
}

void tapCombo3(uint8_t mod1, uint8_t mod2, uint8_t key) {
  Keyboard.press(mod1);
  Keyboard.press(mod2);
  Keyboard.press(key);
  delay(5);
  Keyboard.releaseAll();
}

void mediaTap(uint16_t usage) {
#if HAS_HID_PROJECT
  Consumer.write(usage);
#else
  // Fallback when HID-Project is not installed.
  (void)usage;
#endif
}

void scrollStep(bool clockwise) {
  Mouse.move(0, 0, clockwise ? 1 : -1);
}

void volumeStep(bool clockwise) {
#if HAS_HID_PROJECT
  mediaTap(clockwise ? MEDIA_VOLUME_UP : MEDIA_VOLUME_DOWN);
#else
  tapKey(clockwise ? KEY_PAGE_UP : KEY_PAGE_DOWN);
#endif
}

void sendLayerKey(uint8_t layer, uint8_t keyIndex) {
  switch (layer) {
    case L_BASE:
      switch (keyIndex) {
        case 0: tapCombo2(KEY_LEFT_GUI, KEY_TAB); break;
        case 1: tapKey(KEY_UP_ARROW); break;
        case 2: tapCombo2(KEY_LEFT_ALT, KEY_TAB); break;
        case 3: tapKey(KEY_LEFT_ARROW); break;
        case 4: tapKey(KEY_RETURN); break;
        case 5: tapKey(KEY_RIGHT_ARROW); break;
        case 6: tapCombo2(KEY_LEFT_CTRL, 'z'); break;
        case 7: tapKey(KEY_DOWN_ARROW); break;
        case 8: tapCombo2(KEY_LEFT_CTRL, 'r'); break;
      }
      break;

    case L_EDIT:
      switch (keyIndex) {
        case 0: tapCombo2(KEY_LEFT_CTRL, 'a'); break;
        case 1: tapCombo2(KEY_LEFT_CTRL, 'c'); break;
        case 2: tapCombo2(KEY_LEFT_CTRL, 'v'); break;
        case 3: tapCombo2(KEY_LEFT_CTRL, 'x'); break;
        case 4: tapCombo2(KEY_LEFT_CTRL, KEY_RETURN); break;
        case 6: tapCombo3(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'z'); break;
        case 7: tapKey(' '); break;
        case 8: tapKey(KEY_BACKSPACE); break;
      }
      break;

    case L_MEDIA:
      switch (keyIndex) {
        case 0: mediaTap(MEDIA_PREVIOUS); break;
        case 1: mediaTap(MEDIA_PLAY_PAUSE); break;
        case 2: mediaTap(MEDIA_NEXT); break;
        case 3: mediaTap(MEDIA_REWIND); break;
        case 4: mediaTap(MEDIA_PLAY_PAUSE); break;
        case 5: mediaTap(MEDIA_FAST_FORWARD); break;
        case 6: tapKey(KEY_DOWN_ARROW); break;
        case 7: mediaTap(MEDIA_STOP); break;
        case 8: tapKey(KEY_UP_ARROW); break;
      }
      break;

    case L_FN:
      switch (keyIndex) {
        case 0: tapKey(KEY_F13); break;
        case 1: tapKey(KEY_F14); break;
        case 2: tapKey(KEY_F15); break;
        case 3: tapKey(KEY_F16); break;
        case 4: tapKey(KEY_F17); break;
        case 5: tapKey(KEY_F18); break;
        case 6: tapKey(KEY_F19); break;
        case 7: tapKey(KEY_F20); break;
        case 8: tapKey(KEY_F21); break;
      }
      break;

    case L_RGB:
      switch (keyIndex) {
        case 0: rgbSpeed += 1; break;
        case 1: rgbSpeed = max<int16_t>(1, rgbSpeed - 1); break;
        case 2: rgbOn = !rgbOn; break;
        case 3: rgbHue = (uint8_t)(rgbHue + 8); break;
        case 4: rgbHue = (uint8_t)(rgbHue - 8); break;
        case 5: rgbVal = min<uint8_t>(255, rgbVal + 8); break;
        case 6: rgbSat = min<uint8_t>(255, rgbSat + 8); break;
        case 7: rgbSat = max<uint8_t>(0, rgbSat - 8); break;
        case 8: rgbVal = max<uint8_t>(0, rgbVal - 8); break;
      }
      applyRgb();
      break;

    case L_SELECT:
      switch (keyIndex) {
        case 0: activeLayer = L_EDIT; break;
        case 1: activeLayer = L_MEDIA; break;
        case 2: activeLayer = L_FN; break;
        case 3: activeLayer = L_RGB; break;
        case 4: activeLayer = L_BASE; break;
        case 6: activeLayer = L_SYS; break;
        case 7: activeLayer = L_NAV; break;
        case 8: activeLayer = L_NUM; break;
      }
      applyRgb();
      break;

    case L_SYS:
      switch (keyIndex) {
        case 0: tapCombo2(KEY_LEFT_CTRL, 's'); break;
        case 1: tapCombo2(KEY_LEFT_CTRL, 'c'); break;
        case 2: tapCombo2(KEY_LEFT_CTRL, 'v'); break;
        case 3: tapCombo2(KEY_LEFT_CTRL, 'x'); break;
        case 4: tapCombo2(KEY_LEFT_CTRL, 'z'); break;
        case 5: tapCombo3(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'z'); break;
        case 6: tapKey(KEY_PRINT_SCREEN); break;
        case 7: tapKey(KEY_DELETE); break;
        case 8: tapKey(KEY_ESC); break;
      }
      break;

    case L_NAV:
      switch (keyIndex) {
        case 0: tapKey(KEY_HOME); break;
        case 1: tapKey(KEY_PAGE_UP); break;
        case 2: tapKey(KEY_END); break;
        case 3: tapKey(KEY_LEFT_ARROW); break;
        case 4: tapKey(KEY_RETURN); break;
        case 5: tapKey(KEY_RIGHT_ARROW); break;
        case 6: tapKey(KEY_INSERT); break;
        case 7: tapKey(KEY_PAGE_DOWN); break;
        case 8: tapKey(KEY_DELETE); break;
      }
      break;

    case L_NUM:
      switch (keyIndex) {
        case 0: tapKey('7'); break;
        case 1: tapKey('8'); break;
        case 2: tapKey('9'); break;
        case 3: tapKey('4'); break;
        case 4: tapKey('5'); break;
        case 5: tapKey('6'); break;
        case 6: tapKey('1'); break;
        case 7: tapKey('2'); break;
        case 8: tapKey('3'); break;
      }
      break;
  }
}

void handleEncoderClick() {
  switch (activeLayer) {
    case L_BASE:
      pendingLayer = L_BASE;
      activeLayer = L_SELECT;
      break;
    case L_EDIT:
      tapCombo2(KEY_LEFT_CTRL, 's');
      break;
    case L_MEDIA:
#if HAS_HID_PROJECT
      mediaTap(MEDIA_VOLUME_MUTE);
#endif
      break;
    case L_FN:
      tapKey(KEY_F23);
      break;
    case L_RGB:
      rgbOn = !rgbOn;
      break;
    case L_SELECT:
      activeLayer = pendingLayer;
      break;
    default:
      activeLayer = L_SELECT;
      break;
  }
  applyRgb();
}

void handleEncoderTurn(bool clockwise) {
  if (btnPressed) {
    btnTurned = true;
    activeLayer = (clockwise) ? (uint8_t)((activeLayer + 1) % L_COUNT)
                              : (uint8_t)((activeLayer + L_COUNT - 1) % L_COUNT);
    applyRgb();
    return;
  }

  switch (activeLayer) {
    case L_BASE:
    case L_EDIT:
    case L_FN:
    case L_SYS:
    case L_NAV:
    case L_NUM:
      scrollStep(clockwise);
      break;
    case L_MEDIA:
      volumeStep(clockwise);
      break;
    case L_RGB:
      if (clockwise) {
        rgbVal = min<uint8_t>(255, rgbVal + 8);
      } else {
        rgbVal = max<uint8_t>(0, rgbVal - 8);
      }
      applyRgb();
      break;
    case L_SELECT:
      if (clockwise) {
        pendingLayer = (pendingLayer + 1) % L_COUNT;
      } else {
        pendingLayer = (pendingLayer + L_COUNT - 1) % L_COUNT;
      }
      break;
  }
}

void scanMatrix() {
  for (uint8_t c = 0; c < 3; c++) {
    digitalWrite(COL_PINS[c], HIGH);
    delayMicroseconds(30);

    for (uint8_t r = 0; r < 3; r++) {
      uint8_t index = r * 3 + c;
      bool pressed = digitalRead(ROW_PINS[r]) == HIGH;

      if (pressed && !keyDown[index]) {
        keyDown[index] = true;
        sendLayerKey(activeLayer, index);
      } else if (!pressed && keyDown[index]) {
        keyDown[index] = false;
      }
    }

    digitalWrite(COL_PINS[c], LOW);
  }
}

void setup() {
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(ROW_PINS[i], INPUT_PULLDOWN);
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], LOW);
  }

  pinMode(ENC_BTN_PIN, INPUT_PULLUP);

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  pixels.begin();
  pixels.setBrightness(255);

  Keyboard.begin();
  Mouse.begin();
#if HAS_HID_PROJECT
  Consumer.begin();
#endif

  applyRgb();
  drawStatus();
  lastEncoderPos = encoder.read() / 4;
}

void loop() {
  scanMatrix();

  bool nowPressed = (digitalRead(ENC_BTN_PIN) == LOW);
  if (nowPressed && !btnPressed) {
    btnPressed = true;
    btnTurned = false;
  } else if (!nowPressed && btnPressed) {
    btnPressed = false;
    if (!btnTurned) {
      handleEncoderClick();
    }
  }

  long pos = encoder.read() / 4;
  long delta = pos - lastEncoderPos;
  if (delta != 0) {
    bool clockwise = delta > 0;
    long steps = abs(delta);
    for (long i = 0; i < steps; i++) {
      handleEncoderTurn(clockwise);
    }
    lastEncoderPos = pos;
  }

  if (millis() - lastDisplayMs >= 120) {
    drawStatus();
    lastDisplayMs = millis();
  }
}
