# Arduino (.ino) version

This folder provides an Arduino implementation for the same hardware profile.

## Files

- `silent_display_3x3.ino` : firmware sketch with 9 layers, encoder, encoder-button, OLED and RGB
- `libraries.txt` : required libraries for Arduino IDE / PlatformIO

## Target

- RP2040 board (tested layout assumptions for Seeed XIAO RP2040 pin naming)

## Features included

- 3x3 matrix scan (col2row)
- EC11 encoder on GP8/GP9
- Encoder button on GP10
- SSD1306 OLED status on I2C (GP0/GP1, 0x3C)
- WS2812 RGB (9 LEDs on GP15)
- 9 layers total (0..8)

## Notes

- Media keys are fully enabled when `HID-Project` is installed.
- Without `HID-Project`, media actions fall back to basic keyboard behavior where possible.
