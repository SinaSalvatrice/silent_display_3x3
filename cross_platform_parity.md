# Cross-Platform Parity (QMK -> ZMK / CircuitPython / Arduino)

Reference source: `keymaps/custom/keymap.c`

## Core behavior parity

| Behavior | QMK reference | ZMK | CircuitPython | Arduino |
|---|---|---|---|---|
| Matrix 3x3 pins | GP2-4 rows, GP5-7 cols, col2row | Yes | Yes | Yes |
| Encoder A/B pins | GP8/GP9 | Yes | Yes | Yes |
| Encoder button | GP10 active-low | Yes (direct kscan) | Yes | Yes |
| OLED | SSD1306 I2C GP0/GP1 | Yes | Yes | Yes |
| RGB pin/count | GP15 / 9 LEDs | Keymap behavior + config | Yes | Yes |
| Layer count | 6 in source | Extended to 9 | Extended to 9 | Extended to 9 |

## Encoder action parity

| Active layer | Rotate | Click | Hold+Rotate |
|---|---|---|---|
| BASE | Scroll | Enter selector | Cycle layers (skip selector) |
| EDIT | Scroll | Ctrl+S | Cycle layers (skip selector) |
| MEDIA | Volume +/- | Mute | Cycle layers (skip selector) |
| FN | Scroll | F23 | Cycle layers (skip selector) |
| RGB | Brightness +/- | RGB toggle | Cycle layers (skip selector) |
| SELECT | Change pending layer | Jump to pending layer | n/a |
| SYS/NAV/NUM (added) | Scroll | Enter selector | Cycle layers (skip selector) |

## Notable limitation

- ZMK cannot reproduce every QMK stateful encoder-button interaction as directly as custom C hooks. The current ZMK implementation is close but not perfectly stateful in the same way as QMK's `matrix_scan_user` + `encoder_update_user` interaction.
