/*
 * 03_led_blink.ino
 * Goal: blink an LED on D7 with a 220 Ohm resistor in series.
 *       This is the same pin VirtualTown uses for the town lights.
 */
constexpr uint8_t PIN_LED = 7;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(PIN_LED, HIGH);
  Serial.println("LED ON");
  delay(1000);
  digitalWrite(PIN_LED, LOW);
  Serial.println("LED OFF");
  delay(1000);
}
