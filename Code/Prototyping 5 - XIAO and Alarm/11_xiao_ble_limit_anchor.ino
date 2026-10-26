#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

// ---- Bluetooth stream format -------------------------------------------
//   0 = CSV    "34.2,31.3,9.5"
//       Required by the Bluefruit Plotter and by the browser monitor page.
//       Machine readable, not pleasant to read.
//   1 = TEXT   "body weight 34.2%, heel 31.3 lb, ball 9.5 lb"
//       Readable in the Bluefruit UART terminal
#define STREAM_TEXT 1

BLEUart bleuart;

float a1 = 2.82e7, b1 = -2.22;   // heel, D1
float a2 = 9.83e7, b2 = -2.34;   // ball, D0

float baselineForce = 0;
float fullWeightForce = 0;

int calStep = -1;   // -1 limit, 0 zero, 1 full, 2 live

unsigned long lastTx = 0;

#define CAL_N 200
float calBuf[CAL_N];

char entryBuf[6];
int entryLen = 0;

#define BUZZ         D6
#define BUZZ_ACTIVE  HIGH
#define BUZZ_IDLE    LOW

#define HYSTERESIS   3.0
#define DEBOUNCE_N   2

float limitPct  = 30.0;
float anchorPct = 100.0;   // % of body weight the 'f' anchor was taken at
bool alarming = false;
int overCount = 0;
bool buzzOn = false;

#define LED_ON  LOW
#define LED_OFF HIGH
#define LED_PIP_MS   40
#define PERIOD_SLOW  2000
#define PERIOD_FAST  300

unsigned long ledLast = 0;
bool ledLit = false;

void ledsOff() {
  digitalWrite(LED_RED, LED_OFF);
  digitalWrite(LED_GREEN, LED_OFF);
  digitalWrite(LED_BLUE, LED_OFF);
}

void updateLed() {
  if (alarming) {
    if (!ledLit) { ledsOff(); digitalWrite(LED_RED, LED_ON); ledLit = true; }
    return;
  }
  int pin;
  unsigned long period;
  if (calStep < 2)                 { pin = LED_RED;   period = PERIOD_FAST; }
  else if (Bluefruit.connected())  { pin = LED_GREEN; period = PERIOD_SLOW; }
  else                             { pin = LED_BLUE;  period = PERIOD_SLOW; }

  unsigned long now = millis();
  if (!ledLit && now - ledLast >= period) {
    ledsOff(); digitalWrite(pin, LED_ON); ledLit = true; ledLast = now;
  } else if (ledLit && now - ledLast >= LED_PIP_MS) {
    ledsOff(); ledLit = false; ledLast = now;
  }
}

float readingToForce(int reading, float a, float b) {
  if (reading >= 1015) return 0;
  if (reading < 1) reading = 1;
  float force = a * pow(reading, b);
  if (force < 0) force = 0;
  return force;
}

int readAvg(int pin) {
  long total = 0;
  for (int i = 0; i < 3; i++) { total += analogRead(pin); }
  return total / 3;
}

float readTotal() {
  return readingToForce(readAvg(D1), a1, b1) + readingToForce(readAvg(D0), a2, b2);
}

void sortBuf() {
  for (int i = 1; i < CAL_N; i++) {
    float k = calBuf[i]; int j = i - 1;
    while (j >= 0 && calBuf[j] > k) { calBuf[j + 1] = calBuf[j]; j--; }
    calBuf[j + 1] = k;
  }
}

int readCmd() {
  if (Serial.available()) return Serial.read();
  if (bleuart.available()) return bleuart.read();
  return -1;
}

void say(const char* s) {
  Serial.println(s);
  if (Bluefruit.connected()) {
    String line = String(s) + "\n";          // one write, not two:
    bleuart.write((const uint8_t*)line.c_str(), line.length());
  }
}

void askLimit() {
  entryLen = 0;
}

void buzzerOff() {
  digitalWrite(BUZZ, BUZZ_IDLE);
  buzzOn = false;
}

void resetAll() {
  calStep = -1;
  baselineForce = 0;
  fullWeightForce = 0;
  alarming = false;
  overCount = 0;
  buzzerOff();
  askLimit();
}

void updateAlarm(float pct) {
  if (calStep < 2) { alarming = false; buzzerOff(); return; }

  if (!alarming) {
    if (pct > limitPct) {
      if (++overCount >= DEBOUNCE_N) { alarming = true; overCount = 0; }
    } else {
      overCount = 0;
    }
  } else {
    if (pct < limitPct - HYSTERESIS) { alarming = false; buzzerOff(); return; }
  }

  if (!alarming) { buzzerOff(); return; }

  if (!buzzOn) { digitalWrite(BUZZ, BUZZ_ACTIVE); buzzOn = true; }
}

void startAdv() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void setup() {
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, BUZZ_IDLE);

  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  ledsOff();

  digitalWrite(LED_RED, LED_ON);   delay(120); ledsOff();
  digitalWrite(LED_GREEN, LED_ON); delay(120); ledsOff();
  digitalWrite(LED_BLUE, LED_ON);  delay(120); ledsOff();
  digitalWrite(BUZZ, BUZZ_ACTIVE); delay(60); digitalWrite(BUZZ, BUZZ_IDLE);

  analogReadResolution(10);
  analogReference(AR_VDD4);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("PWB-Insole");
  bleuart.begin();
  startAdv();

  askLimit();
}

void loop() {
  updateLed();

  // ---- STEP 0: prescribed limit ----
  if (calStep == -1) {
    static unsigned long lastDigit = 0;

    if (entryLen > 0 && millis() - lastDigit > 800) {
      Serial.println();
      int v = atoi(entryBuf);
      entryLen = 0;
      if (v < 5 || v > 100) { askLimit(); return; }
      limitPct = (float)v;
      calStep = 0;
      return;
    }

    int c = readCmd();
    if (c < 0) return;

    if (c >= '0' && c <= '9') {
      if (entryLen < 4) {
        entryBuf[entryLen++] = (char)c;
        entryBuf[entryLen] = '\0';
        lastDigit = millis();
        Serial.print((char)c);
      }
    }
    else if (c == 8 || c == 127) {
      if (entryLen > 0) { entryLen--; entryBuf[entryLen] = '\0'; lastDigit = millis(); }
    }
    else if (c == 'r') { resetAll(); }
    return;
  }

  // ---- CALIBRATION ----
  if (calStep < 2) {
    int c = readCmd();
    if (c < 0) return;

    if (c == 'r') { resetAll(); return; }

    if (calStep == 0 && c == 'z') {
      float sum = 0; int n = 0; unsigned long t = millis();
      while (millis() - t < 2000) { sum += readTotal(); n++; }
      baselineForce = sum / n;
      calStep = 1;
    }

    else if (calStep == 1 && c == 'f') {
      delay(1500);
      for (int i = 0; i < CAL_N; i++) { calBuf[i] = readTotal(); delay(20); }
      sortBuf();

      // MEDIAN, not mean. Standing on one foot you sway, and a power law
      // with exponent -2.22 turns a two-count wiggle into several pounds.
      // Taking the mean of a shifting hold is the bug that made every
      // reading at the lab 37% high.
      float med = calBuf[CAL_N / 2];
      // Anchor at the PRESCRIBED LIMIT, not at full body weight: a partial
      // weight-bearing patient cannot stand at 100% on the injured foot.
      fullWeightForce = med;
      anchorPct = limitPct;

      float span = fullWeightForce - baselineForce;

      // A span near zero means zero and full weight captured the same
      // thing — almost always 'z' or 'f' done in the wrong position. Every
      // later reading divides by this, so a tiny span produces enormous
      // nonsense percentages. Refuse it rather than go live with garbage.
      // The only message left. Silence on failure would look exactly like
      // silence on success until you noticed no numbers were arriving.
      if (span < 3.0) {
        say("bad calibration - start over");
        resetAll();
        return;
      }

      calStep = 2;
    }
    return;
  }

  // ---- COMMANDS ----
  int c = readCmd();
  if (c == 'r') { resetAll(); return; }

  // ---- READ ----
  int heel_avg = readAvg(D1);
  int ball_avg = readAvg(D0);
  float heel_lb = readingToForce(heel_avg, a1, b1);
  float ball_lb = readingToForce(ball_avg, a2, b2);
  float percent = ((heel_lb + ball_lb) - baselineForce) / (fullWeightForce - baselineForce) * anchorPct;

  updateAlarm(percent);

  if (Bluefruit.connected() && millis() - lastTx >= 100) {
    lastTx = millis();
    // ONE packet per update.  Each bleuart.print() is its own BLE
    // notification, and phone terminals print one line per notification -
    // so printing field by field makes a single reading look like six lines.
    // Build the whole line, then write it once.
#if STREAM_TEXT
    String line = "body weight " + String(percent, 1) + "%"
                + ", heel " + String(heel_lb, 1) + " lb"
                + ", ball " + String(ball_lb, 1) + " lb"
                + (alarming ? "   *** OVER LIMIT ***" : "") + "\n";
#else
    String line = String(percent, 1) + "," + String(heel_lb, 1)
                                     + "," + String(ball_lb, 1) + "\n";
#endif
    bleuart.write((const uint8_t*)line.c_str(), line.length());
  }

  // Serial printing is the slowest thing in this loop - about 8 ms for a
  // full line at 115200. Throttling it keeps the alarm responsive.
  static unsigned long lastDbg = 0;
  if (millis() - lastDbg >= 250) {
    lastDbg = millis();
    Serial.print("Heel: "); Serial.print(heel_avg);
    Serial.print(" ("); Serial.print(heel_lb, 1);
    Serial.print(" lb) | Ball: "); Serial.print(ball_avg);
    Serial.print(" ("); Serial.print(ball_lb, 1);
    Serial.print(" lb) | TOTAL: "); Serial.print(percent, 1); Serial.print("%");
    if (alarming) Serial.print("   *** OVER LIMIT ***");
    Serial.println();
  }

  delay(2);
}