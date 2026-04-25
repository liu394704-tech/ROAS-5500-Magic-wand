/*
 * 09_hx712_read.ino
 * Goal: bit-bang the HX712 24-bit ADC and dump raw counts.
 *       Use this to confirm wiring of MPM283 -> HX712 -> Arduino.
 *
 * Wiring:
 *   HX712 VCC  -> Arduino 5V
 *   HX712 GND  -> Arduino GND
 *   HX712 DOUT -> Arduino A1   (used as plain digital input)
 *   HX712 SCK  -> Arduino A2   (used as plain digital output)
 *   MPM283 +OUT/-OUT -> HX712 INA+/INA-
 *
 * NOTE: this is a deliberately tiny driver, NOT a calibrated load cell.
 *       It just proves data is flowing. For real measurements use the
 *       HX711/HX712 Arduino library.
 */
constexpr uint8_t PIN_DOUT = A1;
constexpr uint8_t PIN_SCK  = A2;

void setup() {
  pinMode(PIN_DOUT, INPUT);
  pinMode(PIN_SCK, OUTPUT);
  digitalWrite(PIN_SCK, LOW);
  Serial.begin(115200);
}

long readHX712() {
  while (digitalRead(PIN_DOUT)) { /* wait until DOUT goes LOW = ready */ }

  long value = 0;
  for (uint8_t i = 0; i < 24; i++) {
    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(1);
    value = (value << 1) | digitalRead(PIN_DOUT);
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(1);
  }
  // 25th pulse -> next channel/gain selection (channel A, gain 128)
  digitalWrite(PIN_SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_SCK, LOW);

  if (value & 0x800000) value |= 0xFF000000;   // sign-extend 24 -> 32 bits
  return value;
}

void loop() {
  Serial.println(readHX712());
  delay(200);
}
