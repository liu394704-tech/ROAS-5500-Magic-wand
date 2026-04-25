# VirtualTown_Controller_AllLED

Variant of `VirtualTown_Controller.ino` for the bench setup where **the fan,
the waterfall pump and the buzzer have all been replaced by LED indicator
modules**.  No MOSFETs, no external power supply, no audio output — the
entire build runs from USB. The story is now told purely with light.

## Pin map (must match your wiring)

| Sensor Shield pin | Module     | Role in the story                             |
|-------------------|------------|------------------------------------------------|
| **D5**            | HW-500 LED | "Wind" indicator (was the fan)                 |
| **D6**            | HW-477 LED | "Waterfall" indicator (was the pump)           |
| **D7**            | HW-489 LED | Town lights                                    |
| **D8**            | HW-481 LED | "Audio" indicator (was the buzzer)             |
| **D11**           | VS1838B    | IR receiver — the magic wand input             |
| **SDA / SCL**     | LCD 1602   | Status text (default address `0x27`)           |

## State → LED behaviour

| State    | Town (D7) | Wind (D5) | Water (D6) | Audio (D8)              | LCD line 1 / line 2          |
|----------|-----------|-----------|------------|--------------------------|-------------------------------|
| Morning  | ON        | OFF       | OFF        | 250 ms blink             | `Good Morning,` / `Town!`     |
| Nature   | ON        | ON        | ON         | OFF                      | `Wind & Water On` / `Nature breeze` |
| Festival | ON        | ON        | ON         | blinking 4 Hz            | `Festival Mode!` / `Lights dancing` |
| Night    | OFF       | OFF       | OFF        | OFF                      | `Town is` / `Sleeping...` (clears after 3 s) |

## Inputs (both work simultaneously)

- Real IR remote on D11 — replace `IR_KEY_*` with codes captured by
  `tests/05_ir_dump`.
- Virtual wand — type `1` / `2` / `3` / `0` / `p` then Enter in the
  Serial Monitor (115200 baud, **Newline** ending). Useful before the
  teammate's transmitter codes are known and as a backup during the demo.

## If a module looks inverted

Some HW LED modules are *active-low* (LED on when the pin is LOW). If you
see exactly the opposite behaviour from the table above, change

```cpp
constexpr bool LED_ACTIVE_LOW = false;
```

to `true` near the top of the sketch and re-upload.

## How to verify the wiring before flashing this sketch

Flash `../VirtualTown_Controller/tests/13_led_modules_check/13_led_modules_check.ino`
first. It walks all four LEDs in turn and prints which one is currently
expected to be on. Any mismatch points straight at the module that's mis-wired.
