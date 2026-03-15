#pragma once

// Matrix
#define DIODE_DIRECTION COL2ROW

// Encoder button: active low, internal pull-up
#define ENCODER_BTN_PIN GP10

// RGB defaults
#define RGBLIGHT_LIMIT_VAL 150
#define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_STATIC_LIGHT
#define RGBLIGHT_DEFAULT_HUE 149
#define RGBLIGHT_DEFAULT_SAT 255
#define RGBLIGHT_DEFAULT_VAL 80

// OLED over I2C
#define I2C_DRIVER I2CD1
#define I2C1_SCL_PIN GP1
#define I2C1_SDA_PIN GP0
#define OLED_TIMEOUT 60000