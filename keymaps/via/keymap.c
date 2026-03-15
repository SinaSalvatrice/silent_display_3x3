#include QMK_KEYBOARD_H
#include "gpio.h"

enum layer_names {
    _BASE,
    _EDIT,
    _MEDIA,
    _FN,
    _RGB,
    _SELECT,
    _EXTRA1,
    _EXTRA2,
    _EXTRA3
};

static bool btn_pressed = false;
static bool btn_held_with_turn = false;
static uint8_t pending_layer = _BASE;

#ifdef RGBLIGHT_ENABLE
static bool user_rgb_on = true;
static uint8_t current_val = RGBLIGHT_DEFAULT_VAL;

static uint8_t hue_for_layer(uint8_t layer) {
    switch (layer) {
        case _BASE:   return 149; // teal
        case _EDIT:   return  64; // yellow
        case _MEDIA:  return 170; // blue
        case _FN:     return 213; // purple
        case _RGB:    return   0; // red
        case _SELECT: return  85; // green
        case _EXTRA1: return  30; // orange
        case _EXTRA2: return 110; // cyan
        case _EXTRA3: return 200; // pink
        default:      return 149;
    }
}

static void apply_rgb_for_layer(uint8_t layer) {
    if (!user_rgb_on) {
        rgblight_disable_noeeprom();
        return;
    }

    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_sethsv_noeeprom(hue_for_layer(layer), RGBLIGHT_DEFAULT_SAT, current_val);
}
#endif

void keyboard_post_init_user(void) {
    gpio_set_pin_input_high(ENCODER_BTN_PIN);

#ifdef RGBLIGHT_ENABLE
    apply_rgb_for_layer(_BASE);
#endif
}

layer_state_t layer_state_set_user(layer_state_t state) {
#ifdef RGBLIGHT_ENABLE
    uint8_t layer = get_highest_layer(state | default_layer_state);
    apply_rgb_for_layer(layer);
#endif
    return state;
}

void matrix_scan_user(void) {
    bool is_pressed = (gpio_read_pin(ENCODER_BTN_PIN) == 0);

    if (is_pressed && !btn_pressed) {
        btn_pressed = true;
        btn_held_with_turn = false;
    } else if (!is_pressed && btn_pressed) {
        btn_pressed = false;

        if (!btn_held_with_turn) {
            uint8_t layer = get_highest_layer(layer_state | default_layer_state);

            switch (layer) {
                case _BASE:
                    pending_layer = _BASE;
                    layer_move(_SELECT);
                    break;

                case _EDIT:
                    tap_code16(LCTL(KC_S));
                    break;

                case _MEDIA:
                    tap_code(KC_MUTE);
                    break;

                case _FN:
                    tap_code(KC_F23);
                    break;

                case _RGB:
#ifdef RGBLIGHT_ENABLE
                    user_rgb_on = !user_rgb_on;
                    apply_rgb_for_layer(get_highest_layer(layer_state | default_layer_state));
#endif
                    break;

                case _SELECT:
                    layer_move(pending_layer);
                    break;

                case _EXTRA1:
                    tap_code(KC_F1);
                    break;

                case _EXTRA2:
                    tap_code(KC_F2);
                    break;

                case _EXTRA3:
                    tap_code(KC_F3);
                    break;
            }
        }
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    (void)index;

    uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    if (btn_pressed) {
        btn_held_with_turn = true;

        if (clockwise) {
            uint8_t next = (layer >= _EXTRA3) ? _BASE : (layer + 1);
            layer_move(next);
        } else {
            uint8_t prev = (layer <= _BASE) ? _EXTRA3 : (layer - 1);
            layer_move(prev);
        }
        return false;
    }

    switch (layer) {
        case _BASE:
            clockwise ? tap_code(KC_DOWN) : tap_code(KC_UP);
            break;

        case _EDIT:
            clockwise ? tap_code16(LCTL(KC_Z)) : tap_code16(LCTL(KC_Y));
            break;

        case _MEDIA:
            clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
            break;

        case _FN:
            clockwise ? tap_code(KC_PGDN) : tap_code(KC_PGUP);
            break;

        case _RGB:
#ifdef RGBLIGHT_ENABLE
            if (clockwise) {
                current_val = (current_val <= 247) ? (current_val + 8) : 255;
            } else {
                current_val = (current_val >= 8) ? (current_val - 8) : 0;
            }

            if (user_rgb_on) {
                rgblight_sethsv_noeeprom(hue_for_layer(_RGB), RGBLIGHT_DEFAULT_SAT, current_val);
            }
#endif
            break;

        case _SELECT:
            if (clockwise) {
                pending_layer = (pending_layer >= _EXTRA3) ? _BASE : (pending_layer + 1);
            } else {
                pending_layer = (pending_layer <= _BASE) ? _EXTRA3 : (pending_layer - 1);
            }
            break;

        case _EXTRA1:
            clockwise ? tap_code(KC_RIGHT) : tap_code(KC_LEFT);
            break;

        case _EXTRA2:
            clockwise ? tap_code(KC_2) : tap_code(KC_1);
            break;

        case _EXTRA3:
            clockwise ? tap_code(KC_B) : tap_code(KC_A);
            break;
    }

    return false;
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

static void oled_write_layer_name(uint8_t layer) {
    switch (layer) {
        case _BASE:   oled_write_P(PSTR("Base"), false);   break;
        case _EDIT:   oled_write_P(PSTR("Edit"), false);   break;
        case _MEDIA:  oled_write_P(PSTR("Media"), false);  break;
        case _FN:     oled_write_P(PSTR("Fn"), false);     break;
        case _RGB:    oled_write_P(PSTR("RGB"), false);    break;
        case _SELECT: oled_write_P(PSTR("Select"), false); break;
        case _EXTRA1: oled_write_P(PSTR("Extra1"), false); break;
        case _EXTRA2: oled_write_P(PSTR("Extra2"), false); break;
        case _EXTRA3: oled_write_P(PSTR("Extra3"), false); break;
        default:      oled_write_P(PSTR("???"), false);    break;
    }
}

bool oled_task_user(void) {
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    oled_clear();
    oled_write_ln_P(PSTR("silent 3x3"), false);
    oled_write_P(PSTR("Layer: "), false);
    oled_write_layer_name(layer);
    oled_write_ln_P(PSTR(""), false);

    if (layer == _SELECT) {
        oled_write_P(PSTR("Goto: "), false);
        oled_write_layer_name(pending_layer);
        oled_write_ln_P(PSTR(""), false);
    }

#ifdef RGBLIGHT_ENABLE
    oled_write_P(PSTR("RGB: "), false);
    oled_write_ln_P(user_rgb_on ? PSTR("on") : PSTR("off"), false);
#endif

    return false;
}
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        LGUI(KC_TAB), KC_UP,        LALT(KC_TAB),
        KC_LEFT,      KC_ENT,       KC_RGHT,
        LCTL(KC_Z),   KC_DOWN,      LCTL(KC_R)
    ),

    [_EDIT] = LAYOUT(
        LCTL(KC_A),      LCTL(KC_C),      LCTL(KC_V),
        LCTL(KC_X),      LCTL(KC_S),      KC_DEL,
        LCTL(LSFT(KC_Z)), KC_SPC,         KC_BSPC
    ),

    [_MEDIA] = LAYOUT(
        KC_MPRV, KC_MPLY, KC_MNXT,
        KC_MUTE, KC_MSTP, KC_VOLU,
        KC_DOWN, KC_UP,   KC_VOLD
    ),

    [_FN] = LAYOUT(
        KC_F13, KC_F14, KC_F15,
        KC_F16, KC_F17, KC_F18,
        KC_F19, KC_F20, KC_F21
    ),

    [_RGB] = LAYOUT(
        RGB_SPI, RGB_SPD, RGB_TOG,
        RGB_HUI, RGB_HUD, RGB_VAI,
        RGB_SAI, RGB_SAD, RGB_VAD
    ),

    [_SELECT] = LAYOUT(
        TO(_EDIT),   TO(_MEDIA),  TO(_FN),
        TO(_RGB),    TO(_SELECT), TO(_EXTRA1),
        TO(_EXTRA2), TO(_EXTRA3), TO(_BASE)
    ),

    [_EXTRA1] = LAYOUT(
        KC_F1, KC_F2, KC_F3,
        KC_F4, KC_F5, KC_F6,
        KC_F7, KC_F8, KC_F9
    ),

    [_EXTRA2] = LAYOUT(
        KC_1, KC_2, KC_3,
        KC_4, KC_5, KC_6,
        KC_7, KC_8, KC_9
    ),

    [_EXTRA3] = LAYOUT(
        KC_A, KC_B, KC_C,
        KC_D, KC_E, KC_F,
        KC_G, KC_H, KC_I
    )
};