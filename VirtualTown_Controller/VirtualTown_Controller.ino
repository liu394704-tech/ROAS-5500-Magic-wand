/* =============================================================================
 *  VirtualTown_Controller.ino
 *  -----------------------------------------------------------------------------
 *  Project : Miniature Virtual Town - Magic Wand HCI System
 *  Board   : Arduino Mega 2560
 *  Author  : Embedded Systems / HCI Engineering
 *
 *  Description:
 *      The "Magic Wand" is an IR remote that lets the user wake up different
 *      modules of a miniature town diorama. Each button on the wand triggers
 *      a different storytelling state (Morning, Nature, Festival, Night).
 *
 *      The controller is fully non-blocking: millis() is used everywhere so
 *      that the IR receiver can capture the next wand gesture at any moment,
 *      and the festival melody can play while the system still listens.
 *
 *  HCI Interaction Flow:
 *      Wand Button 1  -> [Morning / Awakening]   - lights + chime
 *      Wand Button 2  -> [Nature / Breeze]       - fan + waterfall
 *      Wand Button 3  -> [Festival / Music]      - nature + melody loop
 *      Wand Button 0  -> [Night / Sleep]         - everything off
 *      Wand POWER key -> [Night / Sleep]         - same as button 0
 *
 *  Hardware Pin Mapping:
 *      IR Receiver        -> D11   (IRremote library)
 *      Town LEDs          -> D7    (digital out)
 *      Wind Fan           -> D5    (PWM)
 *      Waterfall Pump     -> D6    (digital out)
 *      Speaker / Buzzer   -> D8    (tone())
 *      LCD 1602 (I2C)     -> SDA / SCL  (Mega: SDA=20, SCL=21), addr 0x27
 *
 *  Libraries:
 *      - IRremote         (https://github.com/Arduino-IRremote/Arduino-IRremote)
 *      - LiquidCrystal_I2C
 *      - Wire (built-in)
 * ===========================================================================*/

#include <Arduino.h>
#include <Wire.h>
#include <IRremote.hpp>          // IRremote v3+/v4+ uses the .hpp header
#include <LiquidCrystal_I2C.h>

/* -------------------------------------------------------------------------- */
/*                              Pin Definitions                               */
/* -------------------------------------------------------------------------- */
constexpr uint8_t PIN_IR_RECV   = 11;   // IR receiver data pin
constexpr uint8_t PIN_LEDS      = 7;    // Town street / building LEDs
constexpr uint8_t PIN_FAN       = 5;    // PWM-capable: wind simulator
constexpr uint8_t PIN_WATERFALL = 6;    // Waterfall pump (digital on/off)
constexpr uint8_t PIN_SPEAKER   = 8;    // Passive buzzer / small speaker

/* -------------------------------------------------------------------------- */
/*                       LCD 1602 I2C Configuration                           */
/* -------------------------------------------------------------------------- */
// Try 0x27 first; if the screen stays blank, swap to 0x3F.
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* -------------------------------------------------------------------------- */
/*                  IR Remote Codes (PLACEHOLDERS - replace!)                 */
/* -------------------------------------------------------------------------- */
// Open the Serial Monitor at 115200 baud, press a wand button, copy the printed
// hex value, and paste it here to override these placeholders.
constexpr uint32_t IR_KEY_1     = 0xFFFF0001;   // Morning
constexpr uint32_t IR_KEY_2     = 0xFFFF0002;   // Nature
constexpr uint32_t IR_KEY_3     = 0xFFFF0003;   // Festival
constexpr uint32_t IR_KEY_0     = 0xFFFF0000;   // Night
constexpr uint32_t IR_KEY_POWER = 0xFFFFFF00;   // Night (alt)

/* -------------------------------------------------------------------------- */
/*                              State Machine                                 */
/* -------------------------------------------------------------------------- */
enum TownState : uint8_t {
  STATE_IDLE = 0,    // boot screen, nothing playing
  STATE_MORNING,     // lights + chime
  STATE_NATURE,      // wind + waterfall
  STATE_FESTIVAL,    // nature + melody
  STATE_NIGHT        // everything off, "sleeping" message
};

TownState currentState = STATE_IDLE;

/* -------------------------------------------------------------------------- */
/*                         Non-blocking Timing Helpers                        */
/* -------------------------------------------------------------------------- */
// Morning chime: short single tone played without blocking.
unsigned long chimeStartMs   = 0;
constexpr unsigned long CHIME_DURATION_MS = 250;
bool chimeActive = false;

// Night auto-clear: clear LCD 3 seconds after entering night state.
unsigned long nightStartMs   = 0;
constexpr unsigned long NIGHT_CLEAR_MS = 3000;
bool nightCleared = false;

// Festival melody (non-blocking, advances note-by-note via millis()).
struct Note { uint16_t freq; uint16_t durMs; };
const Note festivalMelody[] = {
  { 523, 250 }, // C5
  { 587, 250 }, // D5
  { 659, 250 }, // E5
  { 698, 250 }, // F5
  { 784, 350 }, // G5
  { 698, 250 }, // F5
  { 659, 250 }, // E5
  { 587, 250 }, // D5
  { 523, 400 }, // C5
  {   0, 300 }  // rest before looping
};
constexpr uint8_t MELODY_LEN = sizeof(festivalMelody) / sizeof(festivalMelody[0]);
uint8_t        melodyIndex   = 0;
unsigned long  noteStartMs   = 0;
bool           notePlaying   = false;

/* -------------------------------------------------------------------------- */
/*                       Forward Declarations (modular API)                   */
/* -------------------------------------------------------------------------- */
void handleLights(bool state);
void handleWind(bool state, uint8_t speed = 160);
void handleWaterfall(bool state);
void playChime();
void stopAllSound();
void updateFestivalMelody();
void showLcd(const char *line1, const char *line2 = "");

void enterMorning();
void enterNature();
void enterFestival();
void enterNight();

/* ========================================================================== */
/*                                   SETUP                                    */
/* ========================================================================== */
void setup() {
  Serial.begin(115200);
  Serial.println(F("[VirtualTown] Boot..."));

  pinMode(PIN_LEDS,      OUTPUT);
  pinMode(PIN_FAN,       OUTPUT);
  pinMode(PIN_WATERFALL, OUTPUT);
  pinMode(PIN_SPEAKER,   OUTPUT);

  handleLights(false);
  handleWind(false);
  handleWaterfall(false);
  stopAllSound();

  // I2C LCD bring-up
  Wire.begin();
  lcd.init();
  lcd.backlight();
  showLcd("Virtual Town", "Wave the wand!");

  // Start IR receiver. LED_BUILTIN feedback helps verify wiring during demo.
  IrReceiver.begin(PIN_IR_RECV, ENABLE_LED_FEEDBACK);
  Serial.print(F("[VirtualTown] IR ready on pin "));
  Serial.println(PIN_IR_RECV);

  // Virtual wand: type 1/2/3/0/p in Serial Monitor to fire the same states
  // the real IR remote would. Lets you verify the full pipeline before the
  // teammate's transmitter codes are known, and acts as a backup during demo.
  Serial.println(F("[VirtualWand] Type 1/2/3/0/p + Enter in Serial Monitor"));
  Serial.println(F("              1=Morning  2=Nature  3=Festival  0/p=Night"));
}

// Translate one keyboard character from the Serial Monitor into the same
// state-entry call a real IR button would trigger.
void handleVirtualWandChar(char c) {
  switch (c) {
    case '1': Serial.println(F("[VirtualWand] -> 1")); enterMorning();  break;
    case '2': Serial.println(F("[VirtualWand] -> 2")); enterNature();   break;
    case '3': Serial.println(F("[VirtualWand] -> 3")); enterFestival(); break;
    case '0': Serial.println(F("[VirtualWand] -> 0")); enterNight();    break;
    case 'p':
    case 'P': Serial.println(F("[VirtualWand] -> POWER")); enterNight(); break;
    case '\r':
    case '\n':
    case ' ':                                                            break;
    default:
      Serial.print(F("[VirtualWand] unknown key '"));
      Serial.print(c);
      Serial.println(F("' (use 1/2/3/0/p)"));
      break;
  }
}

/* ========================================================================== */
/*                                    LOOP                                    */
/* ========================================================================== */
void loop() {
  /* ---- 1. Always poll the IR receiver first (non-blocking) -------------- */
  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;

    // Debug print so the user can discover real wand codes and replace
    // the IR_KEY_* placeholders above.
    Serial.print(F("[IR] code = 0x"));
    Serial.println(code, HEX);

    // Some remotes emit a 0xFFFFFFFF "repeat" frame when a key is held;
    // we ignore it so the state does not flicker.
    if (code != 0 && code != 0xFFFFFFFF) {
      switch (code) {
        case IR_KEY_1:                              enterMorning();  break;
        case IR_KEY_2:                              enterNature();   break;
        case IR_KEY_3:                              enterFestival(); break;
        case IR_KEY_0:
        case IR_KEY_POWER:                          enterNight();    break;
        default:
          // Unknown wand gesture - leave a hint on the LCD second line.
          showLcd("Unknown spell", "Try 1/2/3/0");
          break;
      }
    }

    IrReceiver.resume();   // ready for the next wand pulse
  }

  /* ---- 1b. Virtual wand: keyboard fallback over Serial ------------------ */
  while (Serial.available() > 0) {
    handleVirtualWandChar((char)Serial.read());
  }

  /* ---- 2. Service the morning chime (non-blocking) ---------------------- */
  if (chimeActive && (millis() - chimeStartMs >= CHIME_DURATION_MS)) {
    noTone(PIN_SPEAKER);
    chimeActive = false;
  }

  /* ---- 3. Service the festival melody (non-blocking) -------------------- */
  if (currentState == STATE_FESTIVAL) {
    updateFestivalMelody();
  }

  /* ---- 4. Service the "Town is sleeping" auto-clear --------------------- */
  if (currentState == STATE_NIGHT && !nightCleared &&
      (millis() - nightStartMs >= NIGHT_CLEAR_MS)) {
    lcd.clear();
    nightCleared = true;
  }
}

/* ========================================================================== */
/*                            State Entry Functions                           */
/* ========================================================================== */

// State 1 - Morning / Awakening
void enterMorning() {
  Serial.println(F("[State] -> MORNING"));
  currentState = STATE_MORNING;

  handleLights(true);
  handleWind(false);
  handleWaterfall(false);
  showLcd("Good Morning,", "Town!");

  playChime();   // short non-blocking high-frequency cue
}

// State 2 - Nature / Breeze
void enterNature() {
  Serial.println(F("[State] -> NATURE"));
  currentState = STATE_NATURE;

  handleLights(true);          // keep the town visible
  handleWind(true, 160);       // ~63% PWM = gentle breeze
  handleWaterfall(true);
  noTone(PIN_SPEAKER);
  chimeActive = false;

  showLcd("Wind & Water On", "Nature breeze...");
}

// State 3 - Festival / Music (extends Nature)
void enterFestival() {
  Serial.println(F("[State] -> FESTIVAL"));
  currentState = STATE_FESTIVAL;

  handleLights(true);
  handleWind(true, 160);
  handleWaterfall(true);

  // Reset melody playhead so it starts cleanly on every entry.
  melodyIndex = 0;
  notePlaying = false;
  noteStartMs = millis();

  showLcd("Festival Mode!", "Let it sing :)");
}

// State 4 - Night / Sleep
void enterNight() {
  Serial.println(F("[State] -> NIGHT"));
  currentState = STATE_NIGHT;

  handleLights(false);
  handleWind(false);
  handleWaterfall(false);
  stopAllSound();

  showLcd("Town is", "Sleeping...");
  nightStartMs = millis();
  nightCleared = false;        // loop() will blank the LCD after 3 s
}

/* ========================================================================== */
/*                      Modular Hardware Helper Functions                     */
/* ========================================================================== */

// Toggle the town LEDs. Encapsulated so the wiring (active-high vs active-low)
// can be flipped in exactly one place.
void handleLights(bool state) {
  digitalWrite(PIN_LEDS, state ? HIGH : LOW);
}

// Drive the wind fan via PWM. `speed` is 0-255 and only used when state==true.
void handleWind(bool state, uint8_t speed) {
  if (state) {
    analogWrite(PIN_FAN, speed);
  } else {
    analogWrite(PIN_FAN, 0);
    digitalWrite(PIN_FAN, LOW);
  }
}

// Waterfall pump - simple on/off relay or MOSFET.
void handleWaterfall(bool state) {
  digitalWrite(PIN_WATERFALL, state ? HIGH : LOW);
}

// Kick off a short non-blocking "ding" used by the morning state.
void playChime() {
  tone(PIN_SPEAKER, 2000);     // 2 kHz cue tone
  chimeStartMs = millis();
  chimeActive  = true;
}

// Cut every audio source.
void stopAllSound() {
  noTone(PIN_SPEAKER);
  chimeActive = false;
  notePlaying = false;
}

// Advance the festival melody one note at a time without blocking.
void updateFestivalMelody() {
  unsigned long now = millis();
  const Note &n = festivalMelody[melodyIndex];

  if (!notePlaying) {
    if (n.freq > 0) {
      tone(PIN_SPEAKER, n.freq);
    } else {
      noTone(PIN_SPEAKER);     // rest
    }
    noteStartMs = now;
    notePlaying = true;
  } else if (now - noteStartMs >= n.durMs) {
    noTone(PIN_SPEAKER);
    notePlaying = false;
    melodyIndex = (melodyIndex + 1) % MELODY_LEN;   // loop forever
  }
}

// Convenience wrapper that always writes 16 chars per row so leftover glyphs
// from the previous message never linger on the 1602 display.
void showLcd(const char *line1, const char *line2) {
  char buf1[17];
  char buf2[17];
  snprintf(buf1, sizeof(buf1), "%-16s", line1);
  snprintf(buf2, sizeof(buf2), "%-16s", line2);

  lcd.setCursor(0, 0);
  lcd.print(buf1);
  lcd.setCursor(0, 1);
  lcd.print(buf2);
}
