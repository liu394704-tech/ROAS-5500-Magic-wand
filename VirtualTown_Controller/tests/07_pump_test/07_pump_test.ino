/*
 * 07_pump_test.ino
 * Goal: cycle the small submersible pump on D6 (3s on, 3s off).
 *
 * Wiring is IDENTICAL to the fan test, just on D6 with its own MOSFET +
 * its own flyback diode. The external 5V can be SHARED with the fan as
 * long as it can supply >= 1.5 A.
 *
 * !!! SAFETY !!!
 *   Submerge the pump in water BEFORE running this sketch.
 *   Running a submersible pump dry will burn out its windings in seconds.
 */
constexpr uint8_t PIN_PUMP = 6;

void setup() {
  pinMode(PIN_PUMP, OUTPUT);
  Serial.begin(115200);
  Serial.println("Make sure pump is in water!");
}

void loop() {
  Serial.println("PUMP ON");
  digitalWrite(PIN_PUMP, HIGH);
  delay(3000);

  Serial.println("PUMP OFF");
  digitalWrite(PIN_PUMP, LOW);
  delay(3000);
}
