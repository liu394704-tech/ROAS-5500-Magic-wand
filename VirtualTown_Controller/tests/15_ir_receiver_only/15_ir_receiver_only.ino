/*
 * 15_ir_receiver_only.ino
 *
 * Receiver-ONLY diagnostic. Use this with any real IR remote (TV remote,
 * AC remote, the teammate's wand). It does NOT touch IrSender, so it cannot
 * suffer the Mega2560 Timer2 conflict that kills receive after send on a
 * single board.
 *
 * What you should see:
 *   - On-board LED 13 (ENABLE_LED_FEEDBACK) flickers EVERY time any IR
 *     activity is detected, even before any decoding. If LED13 stays dark
 *     when you press a remote -> hardware/wiring problem.
 *   - Serial Monitor @115200 prints one line per recognised key:
 *       [IR] proto=NEC  cmd=0x08  raw=0x......
 *     and one line per unrecognised burst:
 *       [IR] proto=UNKNOWN  rawTicks=23
 *
 * Wiring (no MOSFETs, no external power):
 *   VS1838B OUT  -> Sensor Shield D11 group, S pin
 *   VS1838B GND  -> Sensor Shield D11 group, G pin
 *   VS1838B VCC  -> Sensor Shield D11 group, V pin
 */

#include <IRremote.hpp>

constexpr uint8_t PIN_IR = 11;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("\n=== IR Receiver-Only Diagnostic ==="));
  Serial.println(F("Point any IR remote at the receiver and press buttons."));

  IrReceiver.begin(PIN_IR, ENABLE_LED_FEEDBACK);

  Serial.print(F("Listening on D"));
  Serial.println(PIN_IR);
  Serial.print(F("IRremote version: "));
  Serial.println(VERSION_IRREMOTE);
  Serial.println(F("If on-board LED 13 NEVER flickers when you press a"));
  Serial.println(F("remote, the receiver is wired wrong or unpowered."));
  Serial.println();
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.print(F("[IR] proto="));
    Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));

    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.print(F("  rawTicks="));
      Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);
    } else {
      Serial.print(F("  cmd=0x"));
      Serial.print(IrReceiver.decodedIRData.command, HEX);
      Serial.print(F("  addr=0x"));
      Serial.print(IrReceiver.decodedIRData.address, HEX);
      Serial.print(F("  raw=0x"));
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    }
    IrReceiver.resume();
  }
}
