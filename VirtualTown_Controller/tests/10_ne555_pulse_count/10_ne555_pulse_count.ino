/*
 * 10_ne555_pulse_count.ino
 * Goal: count rising edges on D2 produced by a 555 in astable mode.
 *
 * Build the classic 555 astable circuit:
 *   - 555 pin 1 (GND), pin 8 (VCC=5V), pin 4 (RESET=VCC)
 *   - R1 between VCC and pin 7 (DISCH)
 *   - R2 between pin 7 and pin 6 (THRES) tied to pin 2 (TRIG)
 *   - C1 from pin 2 to GND
 *   - 555 pin 3 (OUT) -> Arduino D2
 *
 * Frequency f = 1.44 / ((R1 + 2*R2) * C1)
 */
constexpr uint8_t PIN_555 = 2;
volatile unsigned long pulseCount = 0;

void onPulse() { pulseCount++; }

void setup() {
  pinMode(PIN_555, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_555), onPulse, RISING);
  Serial.begin(115200);
}

void loop() {
  static unsigned long lastReport = 0;
  if (millis() - lastReport >= 1000) {
    noInterrupts();
    unsigned long n = pulseCount;
    pulseCount = 0;
    interrupts();
    Serial.print("Hz = ");
    Serial.println(n);
    lastReport = millis();
  }
}
