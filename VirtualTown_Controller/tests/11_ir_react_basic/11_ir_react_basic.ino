/*
 * 11_ir_react_basic.ino
 *
 * Goal: Prove the full HCI loop "wand button -> Arduino reacts" using only
 *       the IR receiver, an LED and a passive buzzer/speaker. NO MOSFETs,
 *       NO motors, NO LCD - the safest possible integration test.
 *
 * Wiring on the MEGA2560 Sensor Shield (G=GND, V=5V, S=Signal):
 *   IR receiver  -> D11 group  (OUT->S, GND->G, VCC->V)
 *   LED + 220Ohm -> D7  group  (D7 S -> 220Ohm -> LED(+);  LED(-) -> D7 G)
 *   Buzzer (passive) -> D8 group  (+ -> S, - -> G)
 *
 * How to use:
 *   1. First flash 05_ir_dump.ino, press your wand buttons, write down the
 *      five raw=0x...... values for keys 1, 2, 3, 0 and POWER.
 *   2. Replace the five IR_KEY_* constants below.
 *   3. Flash this sketch and open Serial Monitor at 115200 baud.
 *   4. Press wand buttons and watch the LED / hear the buzzer.
 */

#include <IRremote.hpp>

constexpr uint8_t PIN_IR  = 11;
constexpr uint8_t PIN_LED = 7;
constexpr uint8_t PIN_SPK = 8;

// >>> REPLACE THESE FIVE VALUES with the codes you copied from 05_ir_dump <<<
constexpr uint32_t IR_KEY_1     = 0xFFFF0001;   // -> LED ON only
constexpr uint32_t IR_KEY_2     = 0xFFFF0002;   // -> short BEEP only
constexpr uint32_t IR_KEY_3     = 0xFFFF0003;   // -> LED ON + repeating BEEP
constexpr uint32_t IR_KEY_0     = 0xFFFF0000;   // -> ALL OFF
constexpr uint32_t IR_KEY_POWER = 0xFFFFFF00;   // -> ALL OFF (alt)

enum DemoState : uint8_t {
  DS_OFF = 0,
  DS_LED_ONLY,
  DS_BEEP_ONCE,
  DS_LED_AND_LOOP_BEEP
};

DemoState     state           = DS_OFF;
unsigned long lastNoteToggleMs = 0;
bool          beepNoteOn       = false;
unsigned long beepOnceStartMs  = 0;
bool          beepOnceActive   = false;

void applyState() {
  switch (state) {
    case DS_OFF:
      digitalWrite(PIN_LED, LOW);
      noTone(PIN_SPK);
      beepOnceActive = false;
      break;

    case DS_LED_ONLY:
      digitalWrite(PIN_LED, HIGH);
      noTone(PIN_SPK);
      beepOnceActive = false;
      break;

    case DS_BEEP_ONCE:
      digitalWrite(PIN_LED, LOW);
      tone(PIN_SPK, 2000);          // 2 kHz cue tone (non-blocking start)
      beepOnceStartMs = millis();
      beepOnceActive  = true;
      break;

    case DS_LED_AND_LOOP_BEEP:
      digitalWrite(PIN_LED, HIGH);
      lastNoteToggleMs = millis();
      beepNoteOn       = false;
      break;
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SPK, OUTPUT);
  Serial.begin(115200);
  while (!Serial) {}

  IrReceiver.begin(PIN_IR, ENABLE_LED_FEEDBACK);
  Serial.println(F("[demo] Ready. Press wand buttons."));
  Serial.println(F("[demo] 1=LED  2=BEEP  3=BOTH  0/POWER=OFF"));
}

void loop() {
  /* --- 1. Receive IR --- */
  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print(F("[IR] raw=0x"));
    Serial.println(code, HEX);

    if (code != 0 && code != 0xFFFFFFFF) {
      switch (code) {
        case IR_KEY_1:                       state = DS_LED_ONLY;          applyState(); break;
        case IR_KEY_2:                       state = DS_BEEP_ONCE;         applyState(); break;
        case IR_KEY_3:                       state = DS_LED_AND_LOOP_BEEP; applyState(); break;
        case IR_KEY_0:
        case IR_KEY_POWER:                   state = DS_OFF;               applyState(); break;
        default:                             /* ignore unknown keys */                   break;
      }
    }
    IrReceiver.resume();
  }

  /* --- 2. Stop the single 250 ms beep without using delay() --- */
  if (beepOnceActive && (millis() - beepOnceStartMs >= 250)) {
    noTone(PIN_SPK);
    beepOnceActive = false;
  }

  /* --- 3. Drive a simple repeating beep when in BOTH state --- */
  if (state == DS_LED_AND_LOOP_BEEP) {
    unsigned long now = millis();
    if (now - lastNoteToggleMs >= 250) {
      lastNoteToggleMs = now;
      beepNoteOn = !beepNoteOn;
      if (beepNoteOn) {
        tone(PIN_SPK, 1500);
      } else {
        noTone(PIN_SPK);
      }
    }
  }
}
