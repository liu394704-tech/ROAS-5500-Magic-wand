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
| 11| `11_ir_react_basic` | **Wand -> LED + buzzer reaction**, the safest end-to-end HCI smoke test before adding LCD / fan / pump |
| 12| `12_virtual_signal_pipeline` | **No IR hardware needed** - type 1/2/3/0/p in Serial Monitor to fire the same state transitions. Use this to validate the pipeline before the teammate's transmitter codes are known. See `../VIRTUAL_SIGNAL_GUIDE.md`. |
| 13| `13_led_modules_check` | **All-LED bench setup**: walks HW-500 (D5), HW-477 (D6), HW-489 (D7), HW-481 (D8) one at a time so you can confirm every module is wired correctly before running `VirtualTown_Controller_AllLED`. |
| 14| `14_blink_all_no_serial` | Diagnostic: on-board LED 13 blinks as a heartbeat, D5/D6/D7/D8 all turn ON together for 5 s and OFF together for 5 s. **No Serial Monitor needed** - use this to tell whether the upload actually worked and which modules are mis-wired. |
| 15| `15_ir_receiver_only` | **Receiver-only diagnostic.** No IrSender at all, so it never suffers the Mega Timer2 send-vs-receive conflict. Use this with any real remote to prove the VS1838B is alive. |
| 16| `16_ir_loopback_fixed` | Same-board send + receive that actually works on Mega: emitter on D9 (Mega's OC2B), and `IrReceiver.begin()` is called again after every send to re-arm the timer the way `IrReceiver.start()` does NOT. |

Detailed wiring + safety instructions live in
[`../HARDWARE_GUIDE.md`](../HARDWARE_GUIDE.md).
