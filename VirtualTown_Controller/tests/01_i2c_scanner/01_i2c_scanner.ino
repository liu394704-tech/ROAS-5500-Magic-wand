/*
 * 01_i2c_scanner.ino
 * Goal: confirm the LCD I2C backpack is wired correctly and discover its
 *       address (0x27 or 0x3F are most common). Open Serial Monitor @115200.
 */
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("I2C scanner starting..."));
}

void loop() {
  byte found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("I2C device found at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) Serial.println(F("No I2C devices found. Check SDA/SCL/VCC/GND."));
  Serial.println(F("done"));
  delay(3000);
}
