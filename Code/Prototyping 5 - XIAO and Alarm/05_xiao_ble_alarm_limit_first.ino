#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

BLEUart bleuart;

float a1 = 2.82e7, b1 = -2.22;   // heel, D1
float a2 = 9.83e7, b2 = -2.34;   // ball, D0

float baselineForce = 0;
float fullWeightForce = 0;

// -1 = awaiting limit entry, 0 = awaiting 'z', 1 = awaiting 'f', 2 = live
int calStep = -1;

bool liveMode = true;
unsigned long lastTx = 0;

#define CAL_N 200
float calBuf[CAL_N];

// ---- numeric entry ----
char entryBuf[6];
int entryLen = 0;

// ---- buzzer ----
#define BUZZ         D6
#define BUZZ_ACTIVE  HIGH
#define BUZZ_IDLE    LOW

#define HYSTERESIS   3.0
#define DEBOUNCE_N   2

float limitPct = 30.0;
bool alarming = false;
bool muted = false;
int overCount = 0;
bool buzzOn = false;
unsigned long buzzNext = 0;

// ---- LED ----
#define LED_ON  LOW
#define LED_OFF HIGH
#define PIP_MS       40
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
  } else if (ledLit && now - ledLast >= PIP_MS) {
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
  for (int i = 0; i < 5; i++) { total += analogRead(pin); }
  return total / 5;
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
  if (Bluefruit.connected()) { bleuart.print(s); bleuart.print("\n"); }
}

void sayLimit() {
  Serial.print(">> Limit set to "); Serial.print(limitPct, 0); Serial.println("% of body weight");
  if (Bluefruit.connected()) {
    bleuart.print("limit "); bleuart.print(limitPct, 0); bleuart.print("\n");
  }
}

void askLimit() {
  entryLen = 0;
  say("");
  say("What weight-bearing limit were you prescribed?");
  say("Type the number of percent and press Enter.  e.g.  30");
}

void buzzerOff() {
  digitalWrite(BUZZ, BUZZ_IDLE);
  buzzOn = false;
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

  if (!alarming || muted) { buzzerOff(); return; }

  unsigned long onMs  = 80;
  unsigned long offMs = 220;
  if (pct > limitPct * 1.5) { onMs = 150; offMs = 80; }

  unsigned long now = millis();
  if (now >= buzzNext) {
    buzzOn = !buzzOn;
    digitalWrite(BUZZ, buzzOn ? BUZZ_ACTIVE : BUZZ_IDLE);
    buzzNext = now + (buzzOn ? onMs : offMs);
  }
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

  Serial.println("=== PARTIAL WEIGHT-BEARING INSOLE ===");
  askLimit();
}

void loop() {
  updateLed();

  // ---- STEP 0: enter the prescribed limit ----
  if (calStep == -1) {
    int c = readCmd();
    if (c < 0) return;

    if (c >= '0' && c <= '9') {
      if (entryLen < 4) {
        entryBuf[entryLen++] = (char)c;
        entryBuf[entryLen] = '\0';
        Serial.print((char)c);          // echo so typing is visible
      }
    }
    else if (c == 8 || c == 127) {      // backspace
      if (entryLen > 0) { entryLen--; entryBuf[entryLen] = '\0'; }
    }
    else if (c == '\n' || c == '\r') {
      if (entryLen == 0) return;        // bare Enter, ignore
      Serial.println();
      int v = atoi(entryBuf);
      entryLen = 0;
      if (v < 5 || v > 150) {
        say("!! Enter a number between 5 and 150.");
        askLimit();
        return;
      }
      limitPct = (float)v;
      sayLimit();
      say("");
      say("STEP 1: Shoe on and laced. LIFT the foot completely. Send 'z'.");
      calStep = 0;
    }
    return;
  }

  // ---- CALIBRATION ----
  if (calStep < 2) {
    int c = readCmd();
    if (c < 0) return;

    if (calStep == 0 && c == 'z') {
      float sum = 0; int n = 0; unsigned long t = millis();
      while (millis() - t < 2000) { sum += readTotal(); n++; }
      baselineForce = sum / n;
      Serial.print(">> Zero point = "); Serial.print(baselineForce, 1); Serial.println(" lb");
      if (Bluefruit.connected()) { bleuart.print("baseline "); bleuart.print(baselineForce, 1); bleuart.print("\n"); }
      say("STEP 2: Stand on that foot ALONE. Hold still. Send 'f'.");
      calStep = 1;
    }

    else if (calStep == 1 && c == 'f') {
      say(">> Get steady...");
      delay(1500);
      say(">> Sampling 4 sec - hold still");
      for (int i = 0; i < CAL_N; i++) { calBuf[i] = readTotal(); delay(20); }
      sortBuf();

      // MEDIAN, not mean. Standing on one foot you sway, and a power law
      // with exponent -2.22 turns a two-count wiggle into several pounds.
      // Taking the mean of a shifting hold is the bug that made every
      // reading at the lab 37% high.
      float med = calBuf[CAL_N / 2];
      float p10 = calBuf[CAL_N / 10];
      float p90 = calBuf[(CAL_N * 9) / 10];
      float spread = (med > 0) ? (p90 - p10) / med : 1.0;

      Serial.print(">> median "); Serial.print(med, 1);
      Serial.print(" lb | spread "); Serial.print(spread * 100.0, 0); Serial.println("%");
      if (spread > 0.35) say("!! Very unsteady - saved anyway. 'r' to redo.");

      fullWeightForce = med / 0.98;   // ~2% stays on the lifted leg
      if (fullWeightForce - baselineForce < 1) fullWeightForce = baselineForce + 1;

      Serial.print(">> Full weight = "); Serial.print(fullWeightForce, 1); Serial.println(" lb");
      if (Bluefruit.connected()) { bleuart.print("fullweight "); bleuart.print(fullWeightForce, 1); bleuart.print("\n"); }
      say("");
      sayLimit();
      say("=== LIVE - alarm armed ===");
      calStep = 2;
    }
    return;
  }

  // ---- COMMANDS ----
  int c = readCmd();
  if (c == 's') { liveMode = false; say(">> STOPPED"); }
  else if (c == 'c') { liveMode = true; say(">> LIVE"); }
  else if (c == 'r') { calStep = 0; liveMode = true; alarming = false; buzzerOff();
                       say(">> RECALIBRATE: foot unloaded, send 'z'."); return; }
  else if (c == 'l') { calStep = -1; alarming = false; buzzerOff();
                       askLimit(); return; }
  else if (c == '+') { limitPct += 5; if (limitPct > 150) limitPct = 150; sayLimit(); }
  else if (c == '-') { limitPct -= 5; if (limitPct < 5) limitPct = 5; sayLimit(); }
  else if (c == 'm') { muted = !muted; if (muted) buzzerOff();
                       say(muted ? ">> ALARM MUTED" : ">> ALARM ON"); }
  else if (c == '?') {
    sayLimit();
    Serial.print(">> zero "); Serial.print(baselineForce, 1);
    Serial.print(" lb | full "); Serial.print(fullWeightForce, 1);
    Serial.print(" lb | span "); Serial.println(fullWeightForce - baselineForce, 1);
  }

  // ---- READ ----
  int heel_avg = readAvg(D1);
  int ball_avg = readAvg(D0);
  float heel_lb = readingToForce(heel_avg, a1, b1);
  float ball_lb = readingToForce(ball_avg, a2, b2);
  float percent = ((heel_lb + ball_lb) - baselineForce) / (fullWeightForce - baselineForce) * 100.0;

  updateAlarm(percent);

  if (Bluefruit.connected() && millis() - lastTx >= 100) {
    lastTx = millis();
    bleuart.print(percent, 1); bleuart.print(",");
    bleuart.print(heel_lb, 1);  bleuart.print(",");
    bleuart.print(ball_lb, 1);  bleuart.print("\n");
  }

  if (liveMode) {
    Serial.print("Heel: "); Serial.print(heel_avg);
    Serial.print(" ("); Serial.print(heel_lb, 1);
    Serial.print(" lb) | Ball: "); Serial.print(ball_avg);
    Serial.print(" ("); Serial.print(ball_lb, 1);
    Serial.print(" lb) | TOTAL: "); Serial.print(percent, 1); Serial.print("%");
    if (alarming) Serial.print("   *** OVER LIMIT ***");
    Serial.println();
  }

  delay(15);
}
