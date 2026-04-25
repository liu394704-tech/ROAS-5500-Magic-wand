# VirtualTown_Controller

Arduino Mega 2560 firmware for the **Miniature Virtual Town** interactive
diorama. A user waves an IR "magic wand" (any standard IR remote) to switch
the town between four storytelling states: Morning, Nature, Festival and
Night.

## Hardware

| Module                  | Arduino Pin | Notes                            |
|-------------------------|-------------|----------------------------------|
| IR receiver (e.g. VS1838B) | D11      | Uses `IRremote` library          |
| Town LEDs               | D7          | Digital output (HIGH = on)       |
| Wind fan                | D5          | PWM (0-255), via MOSFET / driver |
| Waterfall pump          | D6          | Digital output, via relay/MOSFET |
| Speaker / passive buzzer| D8          | Driven by `tone()`               |
| LCD 1602 I2C            | SDA / SCL   | Mega: SDA=20, SCL=21, addr 0x27  |

> If your LCD stays blank, change `LiquidCrystal_I2C lcd(0x27, 16, 2);`
> to `0x3F` in `VirtualTown_Controller.ino`.

## Required Libraries

Install through the Arduino Library Manager:

- **IRremote** (v3.x or v4.x — provides the `IRremote.hpp` header)
- **LiquidCrystal_I2C** (Frank de Brabander / johnrickman fork)

`Wire` is built into the Arduino core.

## Wand Button → Scene Mapping

| Wand Key | State            | Effect                                                |
|----------|------------------|-------------------------------------------------------|
| `1`      | Morning / Awake  | LEDs on, LCD `Good Morning, Town!`, short 2 kHz chime |
| `2`      | Nature / Breeze  | Fan + waterfall on, LCD `Wind & Water On`             |
| `3`      | Festival / Music | Nature scene + looping melody, LCD `Festival Mode!`   |
| `0` / Power | Night / Sleep | Everything off, LCD `Town is Sleeping...` (clears 3s) |

## Flashing the Real IR Codes

The provided `IR_KEY_*` constants are placeholders (`0xFFFF000X`). To get
your remote's real codes:

1. Open the Arduino IDE Serial Monitor at **115200 baud**.
2. Press a wand button — the firmware prints `[IR] code = 0x...`.
3. Copy that hex value and replace the matching `IR_KEY_*` constant near
   the top of `VirtualTown_Controller.ino`.
4. Re-upload.

## Design Notes

- **Non-blocking**: every timed effect (morning chime, night LCD clear,
  festival melody) is driven by `millis()` so the IR receiver is never
  starved and the wand always feels responsive.
- **Modular helpers**: `handleLights()`, `handleWind()`, `handleWaterfall()`,
  `playChime()`, `updateFestivalMelody()` and `showLcd()` isolate the
  hardware so wiring changes only touch one place.
- **Repeat-frame guard**: IR `0xFFFFFFFF` "key-held" frames are ignored so
  the state does not flicker when the wand button is held down.
