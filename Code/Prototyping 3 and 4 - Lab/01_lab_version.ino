// Two-sensor weight-bearing — LAB DATA COLLECTION (Arduino Uno)
// This is the version used at the Wolf Orthopaedic Biomechanics Laboratory,
// Western University, for the force-plate validation session.
//
// A1 = heel sensor, A0 = ball sensor

float bodyWeight = 177.0;
float a1 = 2.82e7, b1 = -2.22;   // heel, A1
float a2 = 9.83e7, b2 = -2.34;   // ball, A0

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

void setup() {
  Serial.begin(115200);
  Serial.println("=== WEIGHT-BEARING MONITOR (LAB) ===");
  Serial.println("STEP 1: Shoe ON, foot UNLOADED. Type 'z' + Enter.");
}

void loop() {
  // ---- CALIBRATION ----
  if (calStep < 2) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (calStep == 0 && c == 'z') {
        float sum = 0; int n = 0; unsigned long t = millis();
        while (millis() - t < 2000) {
          float fBall = readingToForce(readAvg(A0), a2, b2);
          float fHeel = readingToForce(readAvg(A1), a1, b1);
          sum += (fHeel + fBall); n++;
        }
        baselineForce = sum / n;
        Serial.print(">> Baseline = "); Serial.print(baselineForce, 1); Serial.println(" lb (0%).");
        Serial.println("STEP 2: Stand FULL weight. Type 'f' + Enter, shift 5 sec.");
        calStep = 1;
      }
      else if (calStep == 1 && c == 'f') {
        Serial.println(">> Sampling 5 sec... shift weight!");
        float sum = 0; int n = 0; unsigned long t = millis();
        while (millis() - t < 5000) {
          float fBall = readingToForce(readAvg(A0), a2, b2);
          float fHeel = readingToForce(readAvg(A1), a1, b1);
          sum += (fHeel + fBall); n++;
        }
        fullWeightForce = sum / n;          // <-- THE BUG: mean of a moving hold
        if (fullWeightForce - baselineForce < 1) fullWeightForce = baselineForce + 1;
        Serial.print(">> Full weight = "); Serial.print(fullWeightForce, 1); Serial.println(" lb (100%).");
        Serial.println("=== LIVE. 's'=stop, 'g'=record 5s, 'c'=continue ===");
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
    else if (c == 'g') {
      liveMode = true; recording = true; recordStart = millis();
      Serial.println("====== TRIAL START ======");
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
