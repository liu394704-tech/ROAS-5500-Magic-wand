/*
 * 13_led_modules_check.ino
 *
 * Goal: prove that EACH of the four LED modules (HW-489 on D7, HW-500 on D5,
 *       HW-477 on D6, HW-481 on D8) is wired correctly BEFORE running the
 *       full state machine. The script lights them up one at a time, then
 *       all together, then turns everything off.
 *
 * Wiring on the MEGA2560 Sensor Shield (each LED module has S/V/G):
 *   HW-500 -> D5   (S->D5 S, V->D5 V, G->D5 G)   "wind"
 *   HW-477 -> D6   (S->D6 S, V->D6 V, G->D6 G)   "waterfall"
 *   HW-489 -> D7   (S->D7 S, V->D7 V, G->D7 G)   "town"
 *   HW-481 -> D8   (S->D8 S, V->D8 V, G->D8 G)   "audio cue"
 *
 * Expected behaviour after upload:
 *   Serial Monitor @115200 prints the active LED's name; you should see ONLY
 *   that LED turn on for 1 second, in this order:
 *     "TOWN  (D7)" -> "WIND  (D5)" -> "WATER (D6)" -> "AUDIO (D8)" -> ALL ON
 *     -> ALL OFF, then repeat.
 *
 * If a module never lights:
 *   - check S/V/G are not swapped (read the silk-screen on the module's PCB)
 *   - check the male-male jumper is fully seated
 *   - some HW modules are *active-low* (LED on when pin = LOW) - if you see
 *     the inverse pattern (the one you didn't ask for is the one that lit),
 *     set ACTIVE_LOW to true below and re-upload.
 */

constexpr bool ACTIVE_LOW = false;   // flip to true if your modules are inverted

constexpr uint8_t PIN_TOWN  = 7;     // HW-489
constexpr uint8_t PIN_WIND  = 5;     // HW-500
constexpr uint8_t PIN_WATER = 6;     // HW-477
constexpr uint8_t PIN_AUDIO = 8;     // HW-481

inline void ledWrite(uint8_t pin, bool on) {
  digitalWrite(pin, (on ^ ACTIVE_LOW) ? HIGH : LOW);
}

void allOff() {
  ledWrite(PIN_TOWN,  false);
  ledWrite(PIN_WIND,  false);
  ledWrite(PIN_WATER, false);
  ledWrite(PIN_AUDIO, false);
}

void only(uint8_t pin, const char *name) {
  allOff();
  ledWrite(pin, true);
  Serial.print(F("ON: "));
  Serial.println(name);
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOWN,  OUTPUT);
  pinMode(PIN_WIND,  OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  allOff();
  Serial.println(F("[Check] LED module wiring test"));
}

void loop() {
  only(PIN_TOWN,  "TOWN  (D7, HW-489)");
  only(PIN_WIND,  "WIND  (D5, HW-500)");
  only(PIN_WATER, "WATER (D6, HW-477)");
  only(PIN_AUDIO, "AUDIO (D8, HW-481)");

  Serial.println(F("ON: ALL"));
  ledWrite(PIN_TOWN,  true);
  ledWrite(PIN_WIND,  true);
  ledWrite(PIN_WATER, true);
  ledWrite(PIN_AUDIO, true);
  delay(1500);

  Serial.println(F("OFF"));
  allOff();
  delay(1500);
}
