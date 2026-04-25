/*
 * 05_ir_dump.ino
 * Goal: print every IR code received on pin 11 so you can copy-paste the
 *       real wand-button hex values into VirtualTown_Controller.ino.
 *
 * IMPORTANT: the printed `raw` value is the same field the main controller
 *            compares against (decodedRawData), so use THAT value when
 *            replacing IR_KEY_1 .. IR_KEY_POWER.
 */
#include <IRremote.hpp>

constexpr uint8_t PIN_IR = 11;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  IrReceiver.begin(PIN_IR, ENABLE_LED_FEEDBACK);
  Serial.println(F("Point a remote at the receiver and press buttons..."));
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.print(F("[IR] protocol="));
    Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
    Serial.print(F(", command=0x"));
    Serial.print(IrReceiver.decodedIRData.command, HEX);
    Serial.print(F(", raw=0x"));
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume();
  }
}
