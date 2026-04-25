/*
 * 06_motor_pwm.ino
 * Goal: drive an R130 fan through an IRLZ44N MOSFET on D5.
 *
 * Wiring (see HARDWARE_GUIDE.md section 5):
 *   Fan(+)  -> external +5V
 *   Fan(-)  -> MOSFET Drain
 *   MOSFET Source -> common GND
 *   MOSFET Gate   -> Arduino D5  AND  10kOhm pull-down to GND
 *   1N4007 across the fan, cathode (banded end) to +5V
 *
 * ALWAYS test with the external supply UNPLUGGED first and check the gate
 * voltage with a multimeter before letting current flow through the motor.
 */
constexpr uint8_t PIN_FAN = 5;

void setup() {
  pinMode(PIN_FAN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  Serial.println("OFF");
  analogWrite(PIN_FAN, 0);    delay(2000);

  Serial.println("Slow (PWM 80)");
  analogWrite(PIN_FAN, 80);   delay(2000);

  Serial.println("Medium (PWM 160)");
  analogWrite(PIN_FAN, 160);  delay(2000);

  Serial.println("Full (PWM 255)");
  analogWrite(PIN_FAN, 255);  delay(2000);
}
