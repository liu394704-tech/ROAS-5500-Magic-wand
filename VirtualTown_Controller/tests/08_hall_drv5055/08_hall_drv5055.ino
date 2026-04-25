/*
 * 08_hall_drv5055.ino
 * Goal: read the analog output of a DRV5055A1 linear Hall-effect sensor.
 *
 * Wiring:
 *   DRV5055 pin 1 (VCC)  -> Arduino 5V
 *   DRV5055 pin 2 (GND)  -> Arduino GND
 *   DRV5055 pin 3 (Vout) -> Arduino A0
 *
 * With no magnet present, Vout sits near VCC/2 (~2.5 V).
 * Bring a magnet close: voltage swings up or down depending on pole.
 */
constexpr uint8_t PIN_HALL = A0;

void setup() {
  Serial.begin(115200);
  analogReference(DEFAULT);
}

void loop() {
  int raw = analogRead(PIN_HALL);
  float volts = raw * (5.0f / 1023.0f);
  Serial.print("raw=");
  Serial.print(raw);
  Serial.print("  V=");
  Serial.println(volts, 3);
  delay(200);
}
