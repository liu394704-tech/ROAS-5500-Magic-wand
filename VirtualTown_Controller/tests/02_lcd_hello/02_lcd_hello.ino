/*
 * 02_lcd_hello.ino
 * Goal: print two lines on the 1602 I2C LCD.
 * NOTE: change 0x27 to 0x3F if the I2C scanner reported 0x3F.
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Hello, Town!");
  lcd.setCursor(0, 1);
  lcd.print("LCD OK :)");
}

void loop() {
  // nothing - leave the message on screen
}
