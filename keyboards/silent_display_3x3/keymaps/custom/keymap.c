// --- MATRIX / KEYMAP (3x3) ---
// Assumes your keyboard defines:  LAYOUT(K00,K01,K02, K10,K11,K12, K20,K21,K22)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  /* L0: WINDOW / DESKTOP (Global) */
  [0] = LAYOUT(
    LALT(KC_TAB),                 LGUI(LCTL(KC_LEFT)),            LGUI(LCTL(KC_RIGHT)),
    LALT(LSFT(KC_TAB)),           LGUI(KC_TAB),                   LGUI(LCTL(LSFT(KC_RIGHT))),
    LGUI(LCTL(KC_D)),             LGUI(LCTL(KC_F4)),              LGUI(LCTL(LSFT(KC_LEFT)))
  ),

  /* L1: VSC Core */
  [1] = LAYOUT(
    LCTL(LSFT(KC_P)),             LCTL(KC_P),                     LCTL(LSFT(KC_F)),
    LCTL(KC_GRV),                 LCTL(KC_SLSH),                  LSFT(LALT(KC_F)),
    KC_F5,                        LSFT(KC_F5),                   LCTL(LSFT(KC_B))
  ),

  /* L2: GitHub / Browser */
  [2] = LAYOUT(
    LCTL(KC_T),                   LCTL(KC_W),                     LCTL(LSFT(KC_T)),
    LCTL(KC_L),                   LCTL(KC_TAB),                   LCTL(LSFT(KC_TAB)),
    KC_WWW_BACK,                  KC_WWW_FORWARD,                 LCTL(KC_R)
  ),

  /* L3: QMK / Terminal helpers (generic) */
  [3] = LAYOUT(
    LCTL(LSFT(KC_C)),             LCTL(LSFT(KC_V)),               LCTL(KC_C),
    LCTL(KC_V),                   KC_ENT,                         KC_ESC,
    KC_UP,                        KC_DOWN,                        KC_TAB
  ),

  /* L4: Media */
  [4] = LAYOUT(
    KC_MPRV,                      KC_MPLY,                        KC_MNXT,
    KC_VOLD,                      KC_MUTE,                        KC_VOLU,
    KC_MRWD,                      KC_MSTP,                        KC_MFFD
  ),

  /* L5: Spare / custom */
  [5] = LAYOUT(
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO
  ),

  /* L6: Spare / custom */
  [6] = LAYOUT(
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO
  ),

  /* L7: Spare / custom */
  [7] = LAYOUT(
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO,
    KC_NO,                        KC_NO,                          KC_NO
  ),

  /* L8: LAYER SELECT (tap to jump) */
  [8] = LAYOUT(
    TO(0),                        TO(1),                          TO(2),
    TO(3),                        TO(4),                          TO(5),
    TO(6),                        TO(7),                          KC_NO
  )
};