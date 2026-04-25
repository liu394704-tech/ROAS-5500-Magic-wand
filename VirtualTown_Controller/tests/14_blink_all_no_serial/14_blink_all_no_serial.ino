/*
 * 14_blink_all_no_serial.ino
 *
 * Diagnostic sketch that needs NO Serial Monitor at all.
 *
 * Behaviour:
 *   1. The on-board LED on pin 13 blinks at 1 Hz the entire time.
 *      This is the "heartbeat" - if it is not blinking, the sketch did
 *      not actually upload to the board.
 *   2. D5, D6, D7, D8 are ALL turned ON together for 5 seconds, then ALL
 *      turned OFF together for 5 seconds, in a loop.
 *
 * What to look for on the bench:
 *   - On-board LED 13 NOT blinking -> upload failed; redo Upload, make sure
 *     no other Serial Monitor is holding the port, and that the right Port
 *     is selected (Tools -> Port -> /dev/cu.usbmodemXXXX on Mac).
 *   - On-board LED 13 blinking but NONE of D5/D6/D7/D8 follow the 5s/5s
 *     pattern -> all four modules are mis-wired in the same way (probably V
 *     pin missing, or the modules are active-low).
 *   - On-board LED 13 blinking and SOME modules follow the 5s/5s pattern
 *     but others don't -> the ones that don't follow are individually
 *     mis-wired. Pull each one off, look at its silk-screen, and re-seat
 *     S / V / G against the Sensor Shield's S / V / G.
 *   - HW-477 (7-color LED) will keep cycling colours on its own as long as
 *     it has power. That is the module's hardware doing it, not the code.
 *     What matters is whether it goes COMPLETELY OFF during the 5s "off"
 *     phase. If it stays cycling during the off phase, its V pin is
 *     getting power but its S pin is not connected to D6.
 */

constexpr uint8_t PIN_TOWN  = 7;     // HW-489
constexpr uint8_t PIN_WIND  = 5;     // HW-500
constexpr uint8_t PIN_WATER = 6;     // HW-477
constexpr uint8_t PIN_AUDIO = 8;     // HW-481

constexpr uint8_t PIN_HEARTBEAT = LED_BUILTIN;   // pin 13 on the Mega

unsigned long lastHeartbeatMs = 0;
bool          heartbeatOn     = false;

unsigned long lastModeChangeMs = 0;
bool          modulesOn        = false;

void setAllModules(bool on) {
  digitalWrite(PIN_TOWN,  on ? HIGH : LOW);
  digitalWrite(PIN_WIND,  on ? HIGH : LOW);
  digitalWrite(PIN_WATER, on ? HIGH : LOW);
  digitalWrite(PIN_AUDIO, on ? HIGH : LOW);
}

void setup() {
  pinMode(PIN_TOWN,  OUTPUT);
  pinMode(PIN_WIND,  OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  pinMode(PIN_HEARTBEAT, OUTPUT);

  setAllModules(false);
  digitalWrite(PIN_HEARTBEAT, LOW);

  lastHeartbeatMs  = millis();
  lastModeChangeMs = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastHeartbeatMs >= 500) {
    lastHeartbeatMs = now;
    heartbeatOn = !heartbeatOn;
    digitalWrite(PIN_HEARTBEAT, heartbeatOn ? HIGH : LOW);
  }

  if (now - lastModeChangeMs >= 5000) {
    lastModeChangeMs = now;
    modulesOn = !modulesOn;
    setAllModules(modulesOn);
  }
}
