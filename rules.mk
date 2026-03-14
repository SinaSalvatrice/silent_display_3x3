MCU = RP2040
BOOTLOADER = rp2040
PLATFORM = rp2040
PYTHON = python
CONSOLE_ENABLE    = no
RGBLIGHT_ENABLE   = yes
RGB_MATRIX_ENABLE = no
BACKLIGHT_ENABLE  = no
WS2812_DRIVER     = vendor
MOUSEKEY_ENABLE   = no
EXTRAKEY_ENABLE   = no
ENCODER_ENABLE    = yes
ENCODER_MAP_ENABLE = no
OLED_ENABLE       = yes
OLED_DRIVER       = ssd1306
WEAR_LEVELING_DRIVER = rp2040_flash
QMK_KEYBOARD_H = keyboards/silent_display_3x3/silent_display_3x3.h
SRC += keyboards/silent_display_3x3/silent_display_3x3.h
SRC += keyboards/silent_display_3x3/keymaps/via/keymap.c




