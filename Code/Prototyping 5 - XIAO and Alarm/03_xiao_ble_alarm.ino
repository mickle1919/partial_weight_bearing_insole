#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

BLEUart bleuart;

// ---- calibration curves (fitted on the Uno, see README) ----
float bodyWeight = 177.0;
float a1 = 2.82e7, b1 = -2.22;   // heel, D1
float a2 = 9.83e7, b2 = -2.34;   // ball, D0

float baselineForce = 0;
float fullWeightForce = 0;
int calStep = 0;

bool liveMode = true;
unsigned long lastTx = 0;

#define CAL_N 200
float calBuf[CAL_N];

// ---- alarm ----
#define BUZZ D6
#define HYSTERESIS   3.0     // points below the limit before it releases
#define DEBOUNCE_N   2       // consecutive samples needed to trigger

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
  // Alarm overrides everything: solid red, no blinking to interpret.
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
    ledsOff();
    digitalWrite(pin, LED_ON);
    ledLit = true;
    ledLast = now;
  } else if (ledLit && now - ledLast >= PIP_MS) {
    ledsOff();
    ledLit = false;
    ledLast = now;
  }
}

// ---- sensors ----
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

// ---- comms ----
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
  Serial.print(">> limit "); Serial.print(limitPct, 0); Serial.println("% BW");
  if (Bluefruit.connected()) {
    bleuart.print("limit "); bleuart.print(limitPct, 0); bleuart.print("\n");
  }
}

// ---- the alarm ----
void buzzerOff() {
  digitalWrite(BUZZ, LOW);
  buzzOn = false;
}

void updateAlarm(float pct) {
  // Nothing to compare against until calibration is done.
  if (calStep < 2) { alarming = false; buzzerOff(); return; }

  if (!alarming) {
    if (pct > limitPct) {
      if (++overCount >= DEBOUNCE_N) { alarming = true; overCount = 0; }
    } else {
      overCount = 0;
    }
  } else {
    // release lower than it triggers, so standing on the threshold
    // does not make it chatter
    if (pct < limitPct - HYSTERESIS) { alarming = false; buzzerOff(); return; }
  }

  if (!alarming || muted) { buzzerOff(); return; }

  // Further over the limit -> more insistent. Rhythm carries the
  // severity, since an active buzzer has only one pitch.
  unsigned long onMs  = 80;
  unsigned long offMs = 220;
  if (pct > limitPct * 1.5) { onMs = 150; offMs = 80; }

  unsigned long now = millis();
  if (now >= buzzNext) {
    buzzOn = !buzzOn;
    digitalWrite(BUZZ, buzzOn ? HIGH : LOW);
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
  Serial.begin(115200);

  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  ledsOff();

  // Boot signature: three LED flashes and one short chirp, so you can
  // confirm the device powered up without a laptop.
  digitalWrite(LED_RED, LED_ON);   delay(120); ledsOff();
  digitalWrite(LED_GREEN, LED_ON); delay(120); ledsOff();
  digitalWrite(LED_BLUE, LED_ON);  delay(120); ledsOff();
  digitalWrite(BUZZ, HIGH); delay(60); digitalWrite(BUZZ, LOW);

  analogReadResolution(10);
  analogReference(AR_VDD4);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("PWB-Insole");
  bleuart.begin();
  startAdv();

  Serial.println("=== PWB INSOLE (BLE + ALARM) ===");
  Serial.println("Advertising as 'PWB-Insole'");
  Serial.print("Limit: "); Serial.print(limitPct, 0); Serial.println("% body weight");
  Serial.println("'+'/'-' change limit, 'm' mute, '?' status");
  Serial.println("STEP 1: Shoe ON, foot UNLOADED. Send 'z'.");
}

void loop() {
  updateLed();

  // ---- CALIBRATION ----
  if (calStep < 2) {
    int c = readCmd();
    if (c < 0) return;

    if (calStep == 0 && c == 'z') {
      float sum = 0; int n = 0; unsigned long t = millis();
      while (millis() - t < 2000) { sum += readTotal(); n++; }
      baselineForce = sum / n;
      Serial.print(">> Baseline = "); Serial.print(baselineForce, 1); Serial.println(" lb");
      if (Bluefruit.connected()) { bleuart.print("baseline "); bleuart.print(baselineForce, 1); bleuart.print("\n"); }
      say("STEP 2: Stand on that foot alone, hold still. Send 'f'.");
      calStep = 1;
    }

    else if (calStep == 1 && c == 'f') {
      say(">> Get steady...");
      delay(1500);
      say(">> Sampling 4 sec");
      for (int i = 0; i < CAL_N; i++) { calBuf[i] = readTotal(); delay(20); }
      sortBuf();

      // MEDIAN, not mean. Standing on one foot you sway, and a power law
      // with exponent -2.22 turns a two-count wiggle into several pounds.
      // The median ignores excursions in both directions; the mean does
      // not. Taking the mean of a shifting hold is the bug that made every
      // reading at the lab 37% high.
      float med = calBuf[CAL_N / 2];
      float p10 = calBuf[CAL_N / 10];
      float p90 = calBuf[(CAL_N * 9) / 10];
      float spread = (med > 0) ? (p90 - p10) / med : 1.0;

      Serial.print(">> median "); Serial.print(med, 1);
      Serial.print(" | spread "); Serial.print(spread * 100.0, 0); Serial.println("%");
      if (spread > 0.35) say("!! Very unsteady - saved anyway.");

      fullWeightForce = med / 0.98;   // ~2% of body weight stays on the lifted leg
      if (fullWeightForce - baselineForce < 1) fullWeightForce = baselineForce + 1;

      Serial.print(">> Full weight = "); Serial.print(fullWeightForce, 1); Serial.println(" lb");
      if (Bluefruit.connected()) { bleuart.print("fullweight "); bleuart.print(fullWeightForce, 1); bleuart.print("\n"); }
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
  else if (c == '+') { limitPct += 5; if (limitPct > 150) limitPct = 150; sayLimit(); }
  else if (c == '-') { limitPct -= 5; if (limitPct < 5) limitPct = 5; sayLimit(); }
  else if (c == 'm') { muted = !muted; if (muted) buzzerOff();
                       say(muted ? ">> ALARM MUTED" : ">> ALARM ON"); }
  else if (c == '?') {
    sayLimit();
    Serial.print(">> baseline "); Serial.print(baselineForce, 1);
    Serial.print(" lb | fullWeight "); Serial.print(fullWeightForce, 1);
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
    bleuart.print(percent, 1);
    bleuart.print(",");
    bleuart.print(heel_lb, 1);
    bleuart.print(",");
    bleuart.print(ball_lb, 1);
    bleuart.print("\n");
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
