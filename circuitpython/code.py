import time
import board
import keypad
import rotaryio
import neopixel
import usb_hid
import displayio
import terminalio

from adafruit_display_text import label
import adafruit_displayio_ssd1306
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_hid.mouse import Mouse

# Matrix and peripherals
ROW_PINS = (board.GP2, board.GP3, board.GP4)
COL_PINS = (board.GP5, board.GP6, board.GP7)
ENC_BTN_PIN = board.GP10
ENC_A_PIN = board.GP8
ENC_B_PIN = board.GP9
RGB_PIN = board.GP15
RGB_COUNT = 9

LAYER_NAMES = ["BASE", "EDIT", "MEDIA", "FN", "RGB", "SELECT", "SYS", "NAV", "NUM"]

L_BASE = 0
L_EDIT = 1
L_MEDIA = 2
L_FN = 3
L_RGB = 4
L_SELECT = 5
L_SYS = 6
L_NAV = 7
L_NUM = 8
LAYER_COUNT = 9
CYCLABLE_LAYERS = (L_BASE, L_EDIT, L_MEDIA, L_FN, L_RGB, L_SYS, L_NAV, L_NUM)

active_layer = L_BASE
pending_layer = L_BASE
btn_pressed = False
btn_turned = False

rgb_on = True
rgb_value = 80
rgb_hue = 0
rgb_sat = 255
rgb_speed = 8

# Layer hues in degrees (0-360)
LAYER_HUES = [150, 64, 210, 280, 0, 100, 30, 120, 190]

kbd = Keyboard(usb_hid.devices)
cc = ConsumerControl(usb_hid.devices)
mouse = Mouse(usb_hid.devices)

# Matrix: col2row wiring from QMK means columns are drive/anodes in CP KeyMatrix
matrix = keypad.KeyMatrix(
    row_pins=ROW_PINS,
    column_pins=COL_PINS,
    columns_to_anodes=True,
)
enc_btn = keypad.Keys((ENC_BTN_PIN,), value_when_pressed=False, pull=True)
encoder = rotaryio.IncrementalEncoder(ENC_A_PIN, ENC_B_PIN)
last_enc = encoder.position

pixels = neopixel.NeoPixel(RGB_PIN, RGB_COUNT, auto_write=False)

# Display

displayio.release_displays()
i2c = board.I2C()
display_bus = displayio.I2CDisplay(i2c, device_address=0x3C)
display = adafruit_displayio_ssd1306.SSD1306(display_bus, width=128, height=64)

group = displayio.Group()
text = label.Label(terminalio.FONT, text="init", color=0xFFFFFF, x=0, y=8)
group.append(text)
display.root_group = group


def hsv_to_rgb(h, s, v):
    # h: 0..360, s/v: 0..1
    i = int(h / 60) % 6
    f = (h / 60) - int(h / 60)
    p = v * (1 - s)
    q = v * (1 - f * s)
    t = v * (1 - (1 - f) * s)
    if i == 0:
        r, g, b = v, t, p
    elif i == 1:
        r, g, b = q, v, p
    elif i == 2:
        r, g, b = p, v, t
    elif i == 3:
        r, g, b = p, q, v
    elif i == 4:
        r, g, b = t, p, v
    else:
        r, g, b = v, p, q
    return int(r * 255), int(g * 255), int(b * 255)


def apply_rgb():
    if not rgb_on:
        pixels.fill((0, 0, 0))
        pixels.show()
        return

    if active_layer == L_RGB:
        hue = rgb_hue
        sat = rgb_sat / 255.0
    else:
        hue = LAYER_HUES[active_layer]
        sat = 1.0
    r, g, b = hsv_to_rgb(hue, max(0.0, min(1.0, sat)), max(0.0, min(1.0, rgb_value / 255.0)))
    pixels.fill((r, g, b))
    pixels.show()


def update_display():
    if active_layer == L_SELECT:
        t = "Silent 3x3\nLayer: {}\nGoto: {}\nRGB: {}".format(
            LAYER_NAMES[active_layer], LAYER_NAMES[pending_layer], "ON" if rgb_on else "OFF"
        )
    else:
        t = "Silent 3x3\nLayer: {}\nRGB: {}\nVal: {}".format(
            LAYER_NAMES[active_layer], "ON" if rgb_on else "OFF", rgb_value
        )
    text.text = t


def tap_key(keycode):
    kbd.press(keycode)
    time.sleep(0.01)
    kbd.release_all()


def tap_combo(mod, key):
    kbd.press(mod, key)
    time.sleep(0.01)
    kbd.release_all()


def tap_combo3(mod1, mod2, key):
    kbd.press(mod1, mod2, key)
    time.sleep(0.01)
    kbd.release_all()


def key_action(layer, idx):
    global active_layer, rgb_on, rgb_value, pending_layer, rgb_hue, rgb_sat, rgb_speed

    if layer == L_BASE:
        actions = {
            0: lambda: tap_combo(Keycode.GUI, Keycode.TAB),
            1: lambda: tap_key(Keycode.UP_ARROW),
            2: lambda: tap_combo(Keycode.ALT, Keycode.TAB),
            3: lambda: tap_key(Keycode.LEFT_ARROW),
            4: lambda: tap_key(Keycode.ENTER),
            5: lambda: tap_key(Keycode.RIGHT_ARROW),
            6: lambda: tap_combo(Keycode.CONTROL, Keycode.Z),
            7: lambda: tap_key(Keycode.DOWN_ARROW),
            8: lambda: tap_combo(Keycode.CONTROL, Keycode.R),
        }

    elif layer == L_EDIT:
        actions = {
            0: lambda: tap_combo(Keycode.CONTROL, Keycode.A),
            1: lambda: tap_combo(Keycode.CONTROL, Keycode.C),
            2: lambda: tap_combo(Keycode.CONTROL, Keycode.V),
            3: lambda: tap_combo(Keycode.CONTROL, Keycode.X),
            4: lambda: tap_combo(Keycode.CONTROL, Keycode.ENTER),
            6: lambda: tap_combo3(Keycode.CONTROL, Keycode.SHIFT, Keycode.Z),
            7: lambda: tap_key(Keycode.SPACEBAR),
            8: lambda: tap_key(Keycode.BACKSPACE),
        }

    elif layer == L_MEDIA:
        actions = {
            0: lambda: cc.send(ConsumerControlCode.SCAN_PREVIOUS_TRACK),
            1: lambda: cc.send(ConsumerControlCode.PLAY_PAUSE),
            2: lambda: cc.send(ConsumerControlCode.SCAN_NEXT_TRACK),
            3: lambda: cc.send(ConsumerControlCode.REWIND),
            4: lambda: cc.send(ConsumerControlCode.PLAY_PAUSE),
            5: lambda: cc.send(ConsumerControlCode.FAST_FORWARD),
            6: lambda: tap_key(Keycode.DOWN_ARROW),
            7: lambda: cc.send(ConsumerControlCode.STOP),
            8: lambda: tap_key(Keycode.UP_ARROW),
        }

    elif layer == L_FN:
        actions = {
            0: lambda: tap_key(Keycode.F13),
            1: lambda: tap_key(Keycode.F14),
            2: lambda: tap_key(Keycode.F15),
            3: lambda: tap_key(Keycode.F16),
            4: lambda: tap_key(Keycode.F17),
            5: lambda: tap_key(Keycode.F18),
            6: lambda: tap_key(Keycode.F19),
            7: lambda: tap_key(Keycode.F20),
            8: lambda: tap_key(Keycode.F21),
        }

    elif layer == L_RGB:
        def val_up():
            global rgb_value
            rgb_value = min(255, rgb_value + 8)
            apply_rgb()

        def val_dn():
            global rgb_value
            rgb_value = max(0, rgb_value - 8)
            apply_rgb()

        def hue_up():
            global rgb_hue
            rgb_hue = (rgb_hue + 8) % 360
            apply_rgb()

        def hue_dn():
            global rgb_hue
            rgb_hue = (rgb_hue - 8) % 360
            apply_rgb()

        def sat_up():
            global rgb_sat
            rgb_sat = min(255, rgb_sat + 8)
            apply_rgb()

        def sat_dn():
            global rgb_sat
            rgb_sat = max(0, rgb_sat - 8)
            apply_rgb()

        def spd_up():
            global rgb_speed
            rgb_speed = min(50, rgb_speed + 1)

        def spd_dn():
            global rgb_speed
            rgb_speed = max(1, rgb_speed - 1)

        actions = {
            0: lambda: spd_up(),
            1: lambda: spd_dn(),
            2: lambda: toggle_rgb(),
            3: lambda: hue_up(),
            4: lambda: hue_dn(),
            5: lambda: val_up(),
            6: lambda: sat_up(),
            7: lambda: sat_dn(),
            8: lambda: val_dn(),
        }

    elif layer == L_SELECT:
        actions = {
            0: lambda: set_layer(L_EDIT),
            1: lambda: set_layer(L_MEDIA),
            2: lambda: set_layer(L_FN),
            3: lambda: set_layer(L_RGB),
            4: lambda: set_layer(L_BASE),
            6: lambda: set_layer(L_SYS),
            7: lambda: set_layer(L_NAV),
            8: lambda: set_layer(L_NUM),
        }

    elif layer == L_SYS:
        actions = {
            0: lambda: tap_combo(Keycode.CONTROL, Keycode.S),
            1: lambda: tap_combo(Keycode.CONTROL, Keycode.C),
            2: lambda: tap_combo(Keycode.CONTROL, Keycode.V),
            3: lambda: tap_combo(Keycode.CONTROL, Keycode.X),
            4: lambda: tap_combo(Keycode.CONTROL, Keycode.Z),
            5: lambda: tap_combo3(Keycode.CONTROL, Keycode.SHIFT, Keycode.Z),
            6: lambda: tap_key(Keycode.PRINT_SCREEN),
            7: lambda: tap_key(Keycode.DELETE),
            8: lambda: tap_key(Keycode.ESCAPE),
        }

    elif layer == L_NAV:
        actions = {
            0: lambda: tap_key(Keycode.HOME),
            1: lambda: tap_key(Keycode.PAGE_UP),
            2: lambda: tap_key(Keycode.END),
            3: lambda: tap_key(Keycode.LEFT_ARROW),
            4: lambda: tap_key(Keycode.ENTER),
            5: lambda: tap_key(Keycode.RIGHT_ARROW),
            6: lambda: tap_key(Keycode.INSERT),
            7: lambda: tap_key(Keycode.PAGE_DOWN),
            8: lambda: tap_key(Keycode.DELETE),
        }

    else:  # L_NUM
        actions = {
            0: lambda: tap_key(Keycode.SEVEN),
            1: lambda: tap_key(Keycode.EIGHT),
            2: lambda: tap_key(Keycode.NINE),
            3: lambda: tap_key(Keycode.FOUR),
            4: lambda: tap_key(Keycode.FIVE),
            5: lambda: tap_key(Keycode.SIX),
            6: lambda: tap_key(Keycode.ONE),
            7: lambda: tap_key(Keycode.TWO),
            8: lambda: tap_key(Keycode.THREE),
        }

    act = actions.get(idx)
    if act:
        act()


def set_layer(new_layer):
    global active_layer
    active_layer = new_layer % LAYER_COUNT
    apply_rgb()


def toggle_rgb():
    global rgb_on
    rgb_on = not rgb_on
    apply_rgb()


def next_cyclable_layer(current, clockwise):
    try:
        idx = CYCLABLE_LAYERS.index(current)
    except ValueError:
        return L_BASE

    if clockwise:
        idx = (idx + 1) % len(CYCLABLE_LAYERS)
    else:
        idx = (idx - 1) % len(CYCLABLE_LAYERS)
    return CYCLABLE_LAYERS[idx]


def encoder_click_action():
    global active_layer, pending_layer

    if active_layer == L_BASE:
        pending_layer = 0
        set_layer(L_SELECT)
    elif active_layer == L_EDIT:
        tap_combo(Keycode.CONTROL, Keycode.S)
    elif active_layer == L_MEDIA:
        cc.send(ConsumerControlCode.MUTE)
    elif active_layer == L_FN:
        tap_key(Keycode.F23)
    elif active_layer == L_RGB:
        toggle_rgb()
    elif active_layer == L_SELECT:
        set_layer(pending_layer)
    else:
        pending_layer = active_layer
        set_layer(L_SELECT)


def encoder_turn_action(clockwise):
    global active_layer, pending_layer, rgb_value, btn_turned

    if btn_pressed:
        btn_turned = True
        set_layer(next_cyclable_layer(active_layer, clockwise))
        return

    if active_layer in (L_BASE, L_EDIT, L_FN, L_SYS, L_NAV, L_NUM):
        mouse.move(0, 0, 1 if clockwise else -1)
    elif active_layer == L_MEDIA:
        cc.send(ConsumerControlCode.VOLUME_INCREMENT if clockwise else ConsumerControlCode.VOLUME_DECREMENT)
    elif active_layer == L_RGB:
        rgb_value = min(255, rgb_value + 8) if clockwise else max(0, rgb_value - 8)
        apply_rgb()
    elif active_layer == L_SELECT:
        pending_layer = next_cyclable_layer(pending_layer, clockwise)


apply_rgb()
update_display()

while True:
    event = matrix.events.get()
    if event and event.pressed:
        key_action(active_layer, event.key_number)

    btn_event = enc_btn.events.get()
    if btn_event:
        if btn_event.pressed:
            btn_pressed = True
            btn_turned = False
        elif btn_event.released:
            btn_pressed = False
            if not btn_turned:
                encoder_click_action()

    pos = encoder.position
    if pos != last_enc:
        clockwise = pos > last_enc
        steps = abs(pos - last_enc)
        for _ in range(steps):
            encoder_turn_action(clockwise)
        last_enc = pos

    update_display()
    time.sleep(0.01)
