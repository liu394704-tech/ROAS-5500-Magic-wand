/*
 * 04_buzzer_tone.ino
 * Goal: play do-re-mi-fa-sol on a passive buzzer or 8-Ohm speaker on D8.
 * If you only hear one constant beep regardless, your buzzer is ACTIVE
 * and must be replaced by a PASSIVE one for VirtualTown's festival tune.
 */
constexpr uint8_t PIN_SPK = 8;
const uint16_t notes[] = { 262, 294, 330, 349, 392 }; // C4 D4 E4 F4 G4

void setup() {
  pinMode(PIN_SPK, OUTPUT);
}

void loop() {
  for (uint8_t i = 0; i < 5; i++) {
    tone(PIN_SPK, notes[i], 300);
    delay(350);
  }
  noTone(PIN_SPK);
  delay(1000);
}
