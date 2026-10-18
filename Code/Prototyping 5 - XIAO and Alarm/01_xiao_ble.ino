#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

BLEUart bleuart;

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

// commands may arrive over USB or over BLE
int readCmd() {
  if (Serial.available()) return Serial.read();
  if (bleuart.available()) return bleuart.read();
  return -1;
}

void say(const char* s) {
  Serial.println(s);
  if (Bluefruit.connected()) { bleuart.print(s); bleuart.print("\n"); }
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
  analogReadResolution(10);
  analogReference(AR_VDD4);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("PWB-Insole");
  bleuart.begin();
  startAdv();

  Serial.println("=== PWB INSOLE (BLE) ===");
  Serial.println("Advertising as 'PWB-Insole'");
  Serial.println("STEP 1: Shoe ON, foot UNLOADED. Send 'z'.");
}

void loop() {
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

      // MEDIAN, not mean. Standing on one foot you sway, and the power-law
      // curve turns a two-count wiggle into several pounds. The median
      // ignores excursions in both directions; the mean does not.
      float med = calBuf[CAL_N / 2];
      float p10 = calBuf[CAL_N / 10];
      float p90 = calBuf[(CAL_N * 9) / 10];
      float spread = (med > 0) ? (p90 - p10) / med : 1.0;

      Serial.print(">> median "); Serial.print(med, 1);
      Serial.print(" | spread "); Serial.print(spread * 100.0, 0); Serial.println("%");
      if (spread > 0.35) say("!! Very unsteady - saved anyway.");

      // /0.98 because roughly 2% of body weight stays on the lifted leg
      fullWeightForce = med / 0.98;
      if (fullWeightForce - baselineForce < 1) fullWeightForce = baselineForce + 1;

      Serial.print(">> Full weight = "); Serial.print(fullWeightForce, 1); Serial.println(" lb");
      Serial.print(">> span "); Serial.println(fullWeightForce - baselineForce, 1);
      if (Bluefruit.connected()) { bleuart.print("fullweight "); bleuart.print(fullWeightForce, 1); bleuart.print("\n"); }
      say("=== LIVE ===");
      calStep = 2;
    }
    return;
  }

  // ---- COMMANDS ----
  int c = readCmd();
  if (c == 's') { liveMode = false; say(">> STOPPED"); }
  else if (c == 'c') { liveMode = true; say(">> LIVE"); }
  else if (c == 'r') { calStep = 0; liveMode = true; say(">> RECALIBRATE: foot unloaded, send 'z'."); return; }

  // ---- READ ----
  int heel_avg = readAvg(D1);
  int ball_avg = readAvg(D0);
  float heel_lb = readingToForce(heel_avg, a1, b1);
  float ball_lb = readingToForce(ball_avg, a2, b2);
  float percent = ((heel_lb + ball_lb) - baselineForce) / (fullWeightForce - baselineForce) * 100.0;

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
    Serial.print(" lb) | TOTAL: "); Serial.print(percent, 1); Serial.println("%");
  }

  delay(15);
}
