/* =============================================================================
 *  VirtualTown_Controller_AllLED.ino
 *  -----------------------------------------------------------------------------
 *  Variant of VirtualTown_Controller.ino where the fan, the waterfall pump
 *  and the buzzer have ALL been replaced by LED indicator modules. This is
 *  the safest possible build (no MOSFETs, no external supply, no audio) and
 *  it matches the user's current bench setup:
 *
 *      D5  -> HW-500  ("wind"      indicator LED)
 *      D6  -> HW-477  ("waterfall" indicator LED)
 *      D7  -> HW-489  ("town"      indicator LED)
 *      D8  -> HW-481  ("audio"     indicator LED, replaces the buzzer)
 *      D11 -> IR receiver (VS1838B)
 *      SDA/SCL (20/21) -> 1602 I2C LCD (default 0x27)
 *
 *  State -> LED mapping:
 *      Morning  : town ON,  wind OFF, water OFF, audio = short 250 ms blink
 *      Nature   : town ON,  wind ON,  water ON,  audio OFF
 *      Festival : town ON,  wind ON,  water ON,  audio = blinking 4 Hz
 *      Night    : ALL OFF, LCD shows "Town is Sleeping...", clears after 3 s
 *
 *  Inputs:
 *      - Real IR remote on D11 (replace IR_KEY_* with codes from 05_ir_dump)
 *      - Virtual wand: type 1/2/3/0/p in Serial Monitor (115200, Newline)
 * ===========================================================================*/

#include <Arduino.h>
#include <Wire.h>
#include <IRremote.hpp>
#include <LiquidCrystal_I2C.h>

/* -------------------------------------------------------------------------- */
/*                                Pin map                                     */
/* -------------------------------------------------------------------------- */
constexpr uint8_t PIN_IR_RECV = 11;
constexpr uint8_t PIN_TOWN    = 7;     // HW-489
constexpr uint8_t PIN_WIND    = 5;     // HW-500
constexpr uint8_t PIN_WATER   = 6;     // HW-477
constexpr uint8_t PIN_AUDIO   = 8;     // HW-481

/* If your LED modules light up when pin is LOW (active-low), flip this. */
constexpr bool LED_ACTIVE_LOW = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);    // change to 0x3F if scanner says so
bool lcdReady = false;

/* -------------------------------------------------------------------------- */
/*                       IR codes (replace placeholders)                      */
/* -------------------------------------------------------------------------- */
constexpr uint32_t IR_KEY_1     = 0xFFFF0001;
constexpr uint32_t IR_KEY_2     = 0xFFFF0002;
constexpr uint32_t IR_KEY_3     = 0xFFFF0003;
constexpr uint32_t IR_KEY_0     = 0xFFFF0000;
constexpr uint32_t IR_KEY_POWER = 0xFFFFFF00;

/* -------------------------------------------------------------------------- */
/*                              State machine                                 */
/* -------------------------------------------------------------------------- */
enum TownState : uint8_t { ST_IDLE, ST_MORNING, ST_NATURE, ST_FESTIVAL, ST_NIGHT };
TownState state = ST_IDLE;

/* Non-blocking timing */
unsigned long audioCueStartMs   = 0;
constexpr unsigned long AUDIO_CUE_MS = 250;
bool audioCueActive = false;

unsigned long festivalToggleMs  = 0;
constexpr unsigned long FESTIVAL_BLINK_MS = 125;   // 4 Hz blink (on/off each 125 ms)
bool festivalAudioOn = false;

unsigned long nightStartMs   = 0;
constexpr unsigned long NIGHT_CLEAR_MS = 3000;
bool nightCleared = false;

/* -------------------------------------------------------------------------- */
/*                            Helper functions                                */
/* -------------------------------------------------------------------------- */
inline void ledWrite(uint8_t pin, bool on) {
  digitalWrite(pin, (on ^ LED_ACTIVE_LOW) ? HIGH : LOW);
}

void handleTown (bool on) { ledWrite(PIN_TOWN,  on); }
void handleWind (bool on) { ledWrite(PIN_WIND,  on); }
void handleWater(bool on) { ledWrite(PIN_WATER, on); }
void handleAudio(bool on) { ledWrite(PIN_AUDIO, on); }

void showLcd(const char *l1, const char *l2 = "") {
  if (!lcdReady) return;
  char b1[17], b2[17];
  snprintf(b1, sizeof(b1), "%-16s", l1);
  snprintf(b2, sizeof(b2), "%-16s", l2);
  lcd.setCursor(0, 0); lcd.print(b1);
  lcd.setCursor(0, 1); lcd.print(b2);
}

const char *stateName(TownState s) {
  switch (s) {
    case ST_IDLE:     return "IDLE";
    case ST_MORNING:  return "MORNING";
    case ST_NATURE:   return "NATURE";
    case ST_FESTIVAL: return "FESTIVAL";
    case ST_NIGHT:    return "NIGHT";
  }
  return "?";
}

/* -------------------------------------------------------------------------- */
/*                            State entries                                   */
/* -------------------------------------------------------------------------- */
void enterMorning() {
  Serial.println(F("[State] -> MORNING"));
  state = ST_MORNING;
  handleTown(true); handleWind(false); handleWater(false);
  handleAudio(true);
  audioCueStartMs = millis();
  audioCueActive  = true;
  festivalAudioOn = false;
  showLcd("Good Morning,", "Town!");
}

void enterNature() {
  Serial.println(F("[State] -> NATURE"));
  state = ST_NATURE;
  handleTown(true); handleWind(true); handleWater(true);
  handleAudio(false);
  audioCueActive  = false;
  festivalAudioOn = false;
  showLcd("Wind & Water On", "Nature breeze");
}

void enterFestival() {
  Serial.println(F("[State] -> FESTIVAL"));
  state = ST_FESTIVAL;
  handleTown(true); handleWind(true); handleWater(true);
  audioCueActive  = false;
  festivalAudioOn = true;
  handleAudio(true);
  festivalToggleMs = millis();
  showLcd("Festival Mode!", "Lights dancing");
}

void enterNight() {
  Serial.println(F("[State] -> NIGHT"));
  state = ST_NIGHT;
  handleTown(false); handleWind(false); handleWater(false); handleAudio(false);
  audioCueActive  = false;
  festivalAudioOn = false;
  showLcd("Town is", "Sleeping...");
  nightStartMs = millis();
  nightCleared = false;
}

void handleVirtualWandChar(char c) {
  switch (c) {
    case '1': enterMorning();  break;
    case '2': enterNature();   break;
    case '3': enterFestival(); break;
    case '0': enterNight();    break;
    case 'p': case 'P': enterNight(); break;
    case 'r': case 'R':
      Serial.print(F("[REPORT] state = ")); Serial.println(stateName(state));
      break;
    case '\r': case '\n': case ' ': break;
    default:
      Serial.print(F("[?] unknown '")); Serial.print(c); Serial.println(F("'"));
      break;
  }
}

/* -------------------------------------------------------------------------- */
/*                                 setup()                                    */
/* -------------------------------------------------------------------------- */
void setup() {
  pinMode(PIN_TOWN,  OUTPUT);
  pinMode(PIN_WIND,  OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  handleTown(false); handleWind(false); handleWater(false); handleAudio(false);

  Serial.begin(115200);
  Serial.println(F("[VirtualTown-AllLED] Boot..."));

  Wire.begin();
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() == 0) {
    lcd.init(); lcd.backlight(); lcdReady = true;
    showLcd("Virtual Town", "Wave the wand!");
  } else {
    Serial.println(F("[LCD] not at 0x27 - skipping display"));
  }

  IrReceiver.begin(PIN_IR_RECV, ENABLE_LED_FEEDBACK);
  Serial.print(F("[VirtualTown] IR ready on pin "));
  Serial.println(PIN_IR_RECV);
  Serial.println(F("[VirtualWand] keyboard 1/2/3/0/p also works"));
}

/* -------------------------------------------------------------------------- */
/*                                  loop()                                    */
/* -------------------------------------------------------------------------- */
void loop() {
  /* 1. Real IR */
  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print(F("[IR] code = 0x")); Serial.println(code, HEX);
    if (code != 0 && code != 0xFFFFFFFF) {
      switch (code) {
        case IR_KEY_1:                              enterMorning();  break;
        case IR_KEY_2:                              enterNature();   break;
        case IR_KEY_3:                              enterFestival(); break;
        case IR_KEY_0:
        case IR_KEY_POWER:                          enterNight();    break;
        default:                                                     break;
      }
    }
    IrReceiver.resume();
  }

  /* 2. Virtual wand */
  while (Serial.available() > 0) handleVirtualWandChar((char)Serial.read());

  /* 3. Morning audio cue: short 250 ms blink of HW-481 */
  if (audioCueActive && (millis() - audioCueStartMs >= AUDIO_CUE_MS)) {
    handleAudio(false);
    audioCueActive = false;
  }

  /* 4. Festival audio: blink HW-481 at 4 Hz */
  if (state == ST_FESTIVAL) {
    unsigned long now = millis();
    if (now - festivalToggleMs >= FESTIVAL_BLINK_MS) {
      festivalToggleMs = now;
      festivalAudioOn  = !festivalAudioOn;
      handleAudio(festivalAudioOn);
    }
  }

  /* 5. Night auto-clear LCD after 3 s */
  if (state == ST_NIGHT && !nightCleared &&
      (millis() - nightStartMs >= NIGHT_CLEAR_MS)) {
    if (lcdReady) lcd.clear();
    nightCleared = true;
  }
}
