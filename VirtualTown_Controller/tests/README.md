# VirtualTown — Per-module Test Sketches

Run these in the order below. **Do not** skip ahead to the main controller
until every step here passes.

| # | Sketch | What it proves |
|---|--------|----------------|
| 1 | `01_i2c_scanner` | The LCD backpack is reachable on I2C and you know its address |
| 2 | `02_lcd_hello`   | The 1602 LCD shows characters |
| 3 | `03_led_blink`   | Pin D7 can drive an LED through a 220 Ω resistor |
| 4 | `04_buzzer_tone` | Pin D8 + passive buzzer/speaker can play tones |
| 5 | `05_ir_dump`     | The IR receiver decodes wand presses (**copy these hex codes!**) |
| 6 | `06_motor_pwm`   | A MOSFET on D5 can PWM-drive the R130 fan from an external 5 V supply |
| 7 | `07_pump_test`   | A second MOSFET on D6 can switch the submersible pump |
| 8 | `08_hall_drv5055`| (optional) DRV5055A1 linear Hall sensor reads on A0 |
| 9 | `09_hx712_read`  | (optional) HX712 24-bit ADC streams raw counts |
| 10| `10_ne555_pulse_count` | (optional) Arduino counts a 555 astable's frequency |

Detailed wiring + safety instructions live in
[`../HARDWARE_GUIDE.md`](../HARDWARE_GUIDE.md).
