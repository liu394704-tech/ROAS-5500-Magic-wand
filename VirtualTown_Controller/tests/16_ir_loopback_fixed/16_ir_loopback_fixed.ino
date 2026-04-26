/*
 * 16_ir_loopback_fixed.ino
 *
 * Same intent as your "IR Diag Mode" sketch - send a known IR code from the
 * board and try to receive it on the same board - but this version actually
 * works around the Mega2560's IRremote limitations.
 *
 * KEY FIXES vs the original sketch:
 *   1. The IR-emitting LED MUST be on D9 (Mega's OC2B / Timer2 hardware
 *      PWM pin). Using D3 produces a weak, jittery 38 kHz carrier that
 *      VS1838B usually fails to decode in close-range loop-back.
 *   2. After sending, IrReceiver.start() does NOT re-arm the receive ISR
 *      because IrSender.begin() already stole Timer2. The fix is to call
 *      IrReceiver.begin(...) AGAIN after every send to fully re-init the
 *      receive timer/ISR path.
 *   3. The receiver's AGC stays saturated for ~100 ms after a nearby IR
 *      burst, so we wait 200 ms before re-arming.
 *
 * REMAINING REAL-WORLD LIMITATION:
 *   The emitter LED on D9 must NOT shine directly into the VS1838B at close
 *   range, otherwise the receiver clips and you only see UNKNOWN frames.
 *   Either (a) put a piece of paper between them, (b) point them away from
 *   each other and bounce off a wall ~30 cm away, or (c) just use a real
 *   remote with sketch 15_ir_receiver_only.ino - that always works.
 *
 * Wiring:
 *   VS1838B receiver -> Sensor Shield D11 group (OUT->S, GND->G, VCC->V)
 *   IR emitter LED   -> Sensor Shield D9  group, anode (long leg) to S
 *                       through a 100 Ohm resistor; cathode to G.
 *                       (Or any IR LED + 100 Ohm in series between D9 and GND.)
 *
 * Usage:
 *   - Open Serial Monitor @115200, line ending = "Newline".
 *   - Type 's' + Enter to send the test code once.
 */

#include <IRremote.hpp>

constexpr uint8_t  PIN_IR_RX  = 11;
constexpr uint8_t  PIN_IR_TX  = 9;          // <-- MUST be 9 on Mega
constexpr uint16_t TEST_NEC_ADDR = 0x00;
constexpr uint16_t TEST_NEC_CMD  = 0x42;

void armReceiver() {
  IrReceiver.begin(PIN_IR_RX, ENABLE_LED_FEEDBACK);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("\n=== IR loop-back (fixed for Mega) ==="));
  Serial.println(F("Type 's' + Enter to send a single NEC test frame."));

  IrSender.begin(PIN_IR_TX);  // sets up Timer2 for 38 kHz
  armReceiver();              // re-arms receive AFTER sender setup
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 's' || c == 'S') {
      Serial.println(F(">> sending NEC 0x00/0x42"));
      IrSender.sendNEC(TEST_NEC_ADDR, TEST_NEC_CMD, 0);

      // Wait long enough for our own AGC to settle, then FULLY re-init the
      // receive path - .start() / .resume() are NOT enough on Mega after a
      // send, because IrSender.begin() reconfigured the shared timer.
      delay(200);
      armReceiver();
      Serial.println(F("   receiver re-armed"));
    }
  }

  if (IrReceiver.decode()) {
    Serial.print(F("<< proto="));
    Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println(F("  (saturated/garbled - move emitter farther away)"));
    } else {
      Serial.print(F("  addr=0x")); Serial.print(IrReceiver.decodedIRData.address, HEX);
      Serial.print(F("  cmd=0x"));  Serial.print(IrReceiver.decodedIRData.command, HEX);
      Serial.print(F("  raw=0x"));  Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    }
    IrReceiver.resume();
  }
}
