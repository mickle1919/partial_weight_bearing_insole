// Two-sensor weight-bearing — 'f' CALIBRATION FIX
// A1 = GOOD (heel), A0 = BAD (ball)

float bodyWeight = 177.0;
float a1 = 2.82e7, b1 = -2.22;   // heel (good), A1
float a2 = 9.83e7, b2 = -2.34;   // ball (bad), A0

float baselineForce = 0;
float fullWeightForce = 0;
int calStep = 0;

bool liveMode = true;
bool recording = false;
unsigned long recordStart = 0;

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
  float fBall = readingToForce(readAvg(A0), a2, b2);
  float fHeel = readingToForce(readAvg(A1), a1, b1);
  return fHeel + fBall;
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== WEIGHT-BEARING MONITOR (f-fix) ===");
  Serial.println("STEP 1: Shoe ON, foot UNLOADED (lift it). Type 'z' + Enter.");
}

void loop() {
  // ---- CALIBRATION ----
  if (calStep < 2) {
    if (Serial.available() > 0) {
      char c = Serial.read();

      if (calStep == 0 && c == 'z') {
        float sum = 0; int n = 0; unsigned long t = millis();
        while (millis() - t < 2000) { sum += readTotal(); n++; }
        baselineForce = sum / n;
        Serial.print(">> Baseline = "); Serial.print(baselineForce, 1);
        Serial.println(" lb (0%).");
        Serial.println("STEP 2: Stand on the SENSOR FOOT ONLY, other foot OFF the ground.");
        Serial.println("        Hold STILL - do NOT shift your weight.");
        Serial.println("        Type 'f' + Enter, then hold steady 3 sec.");
        calStep = 1;
      }

      else if (calStep == 1 && c == 'f') {
        Serial.println(">> Get steady...");
        delay(1500);                       // let the wobble settle before sampling
        Serial.println(">> Sampling 3 sec - HOLD STILL");

        float sum = 0; int n = 0;
        float mn = 1e9, mx = -1e9;
        unsigned long t = millis();
        while (millis() - t < 3000) {
          float tot = readTotal();
          sum += tot; n++;
          if (tot < mn) mn = tot;
          if (tot > mx) mx = tot;
        }
        float avg = sum / n;
        float spread = (avg > 0) ? (mx - mn) / avg : 1.0;

        Serial.print(">> Held "); Serial.print(avg, 1);
        Serial.print(" lb, spread "); Serial.print(spread * 100.0, 0);
        Serial.println("%");

        if (spread > 0.15) {
          Serial.println("!! TOO MUCH MOVEMENT - that is the old bug. Not saved.");
          Serial.println("!! Hold completely still and type 'f' again.");
          return;                          // stay on calStep 1, let them retry
        }

        fullWeightForce = avg / 0.98;      // ~2% stays on the lifted leg
        if (fullWeightForce - baselineForce < 1) fullWeightForce = baselineForce + 1;

        Serial.print(">> Full weight = "); Serial.print(fullWeightForce, 1);
        Serial.println(" lb (100%).");
        Serial.println(">> SANITY CHECK: expect roughly 55-64 lb.");
        Serial.println(">>   ~48 lb means the fix did not take.");
        Serial.print(">> Baseline "); Serial.print(baselineForce, 1);
        Serial.print(" lb | span "); Serial.print(fullWeightForce - baselineForce, 1);
        Serial.println(" lb");
        Serial.println("=== LIVE. 's'=stop, 'g'=record 5s, 'c'=continue, 'r'=recalibrate ===");
        calStep = 2;
      }
    }
    return;
  }

  // ---- COMMANDS ----
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 's') { liveMode = false; recording = false; Serial.println(">> STOPPED"); }
    else if (c == 'c') { liveMode = true; recording = false; Serial.println(">> LIVE"); }
    else if (c == 'r') {
      calStep = 0; liveMode = true; recording = false;
      Serial.println(">> RECALIBRATE: foot UNLOADED, type 'z'.");
      return;
    }
    else if (c == 'g') {
      liveMode = true; recording = true; recordStart = millis();
      Serial.println("====== TRIAL START ======");
      Serial.print("# baseline_lb="); Serial.print(baselineForce, 2);
      Serial.print(" fullWeight_lb="); Serial.println(fullWeightForce, 2);
      Serial.println("Time(ms),Heel_avg,Heel_lb,Ball_avg,Ball_lb,Percent,Heel_raw,Ball_raw");
    }
  }

  if (!liveMode && !recording) return;

  // ---- READ ----
  int heel_avg = readAvg(A1);
  int ball_avg = readAvg(A0);
  int heel_raw = analogRead(A1);
  int ball_raw = analogRead(A0);

  float heel_lb = readingToForce(heel_avg, a1, b1);
  float ball_lb = readingToForce(ball_avg, a2, b2);
  float totalForce = heel_lb + ball_lb;
  float percent = ((totalForce - baselineForce) / (fullWeightForce - baselineForce)) * 100.0;

  if (recording) {
    unsigned long elapsed = millis() - recordStart;
    Serial.print(elapsed); Serial.print(",");
    Serial.print(heel_avg); Serial.print(",");
    Serial.print(heel_lb, 1); Serial.print(",");
    Serial.print(ball_avg); Serial.print(",");
    Serial.print(ball_lb, 1); Serial.print(",");
    Serial.print(percent, 1); Serial.print(",");
    Serial.print(heel_raw); Serial.print(",");
    Serial.println(ball_raw);
    if (elapsed > 5000) {
      recording = false;
      liveMode = false;
      Serial.println("====== TRIAL END ======");
      Serial.println(">> STOPPED (press 'c' for live, 'g' for another trial)");
    }
    delay(15);
  } else {
    Serial.print("Heel: ");
    Serial.print(heel_avg); Serial.print(", "); Serial.print(heel_raw);
    Serial.print(" ("); Serial.print(heel_lb, 1); Serial.print(" lb) | Ball: ");
    Serial.print(ball_avg); Serial.print(", "); Serial.print(ball_raw);
    Serial.print(" ("); Serial.print(ball_lb, 1); Serial.print(" lb) | TOTAL: ");
    Serial.print(percent, 1); Serial.println("%");
    delay(15);
  }
}
