/*
 * 12_virtual_signal_pipeline.ino
 *
 * Goal: validate the ENTIRE VirtualTown state machine WITHOUT the IR receiver
 *       and WITHOUT knowing the teammate's transmitter codes yet.
 *
 *       You "are" the transmitter: type characters into the Serial Monitor
 *       and they trigger the exact same state transitions that the final
 *       IR remote will trigger later. This is classic stub/mock testing.
 *
 * Recommended hookup (any subset works - missing peripherals are harmless):
 *   LED + 220Ohm  -> Sensor Shield D7  (S = via 220Ohm to LED+, G = LED-)
 *   Passive buzzer-> Sensor Shield D8  (+ -> S, - -> G)
 *   1602 I2C LCD  -> SDA(20)/SCL(21) on the shield's top-right 4-pin block
 *   (Fan + pump are intentionally OFF in this sketch - safety first.)
 *
 * Usage:
 *   1. Upload this sketch.
 *   2. Open Serial Monitor at 115200, line ending = "Newline".
 *   3. Type a single character then Enter:
 *         1 -> Morning   (LED on, short chime, LCD "Good Morning, Town!")
 *         2 -> Nature    (LED on, fake fan/pump prints, LCD "Wind & Water On")
 *         3 -> Festival  (LED on, looping melody, LCD "Festival Mode!")
 *         0 -> Night     (everything off, LCD "Town is Sleeping...")
 *         p -> Night (POWER alias)
 *         r -> print current state (debug)
 *         h -> help
 *   4. When the teammate hands you the real IR codes later, just paste them
 *      into VirtualTown_Controller.ino - this stub will already have proven
 *      the rest of the system works.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

constexpr uint8_t PIN_LED = 7;
constexpr uint8_t PIN_SPK = 8;
// Fake "actuator" pins - we ONLY print to Serial here, we don't drive them,
// because in this stub we may not have MOSFETs/motors wired yet.
constexpr uint8_t PIN_FAN_FAKE  = 5;
constexpr uint8_t PIN_PUMP_FAKE = 6;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // change to 0x3F if your scanner says so
bool lcdReady = false;

enum State : uint8_t { ST_IDLE, ST_MORNING, ST_NATURE, ST_FESTIVAL, ST_NIGHT };
State state = ST_IDLE;

// Non-blocking timers (mirror the real controller exactly)
unsigned long chimeStartMs = 0; bool chimeActive = false;
unsigned long nightStartMs = 0; bool nightCleared = false;
struct Note { uint16_t f; uint16_t d; };
const Note melody[] = {
  {523,250},{587,250},{659,250},{698,250},{784,350},
  {698,250},{659,250},{587,250},{523,400},{0,300}
};
constexpr uint8_t MELODY_LEN = sizeof(melody)/sizeof(melody[0]);
uint8_t       mIdx = 0;
unsigned long noteStartMs = 0;
bool          notePlaying = false;

void showLcd(const char *l1, const char *l2 = "") {
  if (!lcdReady) return;
  char b1[17], b2[17];
  snprintf(b1, sizeof(b1), "%-16s", l1);
  snprintf(b2, sizeof(b2), "%-16s", l2);
  lcd.setCursor(0, 0); lcd.print(b1);
  lcd.setCursor(0, 1); lcd.print(b2);
}

const char *stateName(State s) {
  switch (s) {
    case ST_IDLE:     return "IDLE";
    case ST_MORNING:  return "MORNING";
    case ST_NATURE:   return "NATURE";
    case ST_FESTIVAL: return "FESTIVAL";
    case ST_NIGHT:    return "NIGHT";
  }
  return "?";
}

void enterMorning() {
  state = ST_MORNING;
  Serial.println(F("[STATE] MORNING (lights on, chime)"));
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_FAN_FAKE,  LOW);
  digitalWrite(PIN_PUMP_FAKE, LOW);
  tone(PIN_SPK, 2000);
  chimeStartMs = millis(); chimeActive = true;
  showLcd("Good Morning,", "Town!");
}

void enterNature() {
  state = ST_NATURE;
  Serial.println(F("[STATE] NATURE (lights+fake-fan+fake-pump)"));
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_FAN_FAKE,  HIGH);   // we just toggle the pin to be visible
  digitalWrite(PIN_PUMP_FAKE, HIGH);   // on a meter; no MOSFET needed yet
  noTone(PIN_SPK); chimeActive = false;
  showLcd("Wind & Water On", "Nature breeze");
}

void enterFestival() {
  state = ST_FESTIVAL;
  Serial.println(F("[STATE] FESTIVAL (lights+fake-fan+fake-pump+melody)"));
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_FAN_FAKE,  HIGH);
  digitalWrite(PIN_PUMP_FAKE, HIGH);
  mIdx = 0; notePlaying = false; noteStartMs = millis();
  showLcd("Festival Mode!", "Let it sing :)");
}

void enterNight() {
  state = ST_NIGHT;
  Serial.println(F("[STATE] NIGHT (all off)"));
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_FAN_FAKE,  LOW);
  digitalWrite(PIN_PUMP_FAKE, LOW);
  noTone(PIN_SPK);
  chimeActive = false; notePlaying = false;
  showLcd("Town is", "Sleeping...");
  nightStartMs = millis(); nightCleared = false;
}

void updateMelody() {
  unsigned long now = millis();
  const Note &n = melody[mIdx];
  if (!notePlaying) {
    if (n.f) tone(PIN_SPK, n.f); else noTone(PIN_SPK);
    noteStartMs = now; notePlaying = true;
  } else if (now - noteStartMs >= n.d) {
    noTone(PIN_SPK); notePlaying = false;
    mIdx = (mIdx + 1) % MELODY_LEN;
  }
}

void printHelp() {
  Serial.println(F("=== Virtual Wand Pipeline ==="));
  Serial.println(F("  1 = Morning   2 = Nature   3 = Festival"));
  Serial.println(F("  0 = Night     p = Power (=Night)"));
  Serial.println(F("  r = report current state    h = this help"));
  Serial.println(F("  Type one char + Enter."));
}

void handleChar(char c) {
  switch (c) {
    case '1': enterMorning();  break;
    case '2': enterNature();   break;
    case '3': enterFestival(); break;
    case '0': enterNight();    break;
    case 'p': case 'P': enterNight(); break;
    case 'r': case 'R':
      Serial.print(F("[REPORT] state = "));
      Serial.println(stateName(state));
      break;
    case 'h': case 'H': printHelp(); break;
    case '\r': case '\n': case ' ':  break;
    default:
      Serial.print(F("[?] unknown char '"));
      Serial.print(c);
      Serial.println(F("' - press h for help"));
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SPK, OUTPUT);
  pinMode(PIN_FAN_FAKE,  OUTPUT);
  pinMode(PIN_PUMP_FAKE, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_FAN_FAKE,  LOW);
  digitalWrite(PIN_PUMP_FAKE, LOW);

  Serial.begin(115200);
  while (!Serial) {}

  Wire.begin();
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() == 0) {
    lcd.init(); lcd.backlight(); lcdReady = true;
    showLcd("Virtual Wand", "Stub ready");
  } else {
    Serial.println(F("[LCD] no device at 0x27 - LCD updates skipped"));
  }

  Serial.println(F("[Boot] virtual signal pipeline online"));
  printHelp();
}

void loop() {
  while (Serial.available() > 0) {
    handleChar((char)Serial.read());
  }

  if (chimeActive && (millis() - chimeStartMs >= 250)) {
    noTone(PIN_SPK);
    chimeActive = false;
  }
  if (state == ST_FESTIVAL) updateMelody();
  if (state == ST_NIGHT && !nightCleared && (millis() - nightStartMs >= 3000)) {
    if (lcdReady) lcd.clear();
    nightCleared = true;
  }
}
