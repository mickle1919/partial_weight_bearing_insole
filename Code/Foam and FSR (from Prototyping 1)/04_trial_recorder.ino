// Prototyping 1 - trial recorder, used for the foam calibration runs.
//
// The foam crept downward under a steady load instead of holding a value,
// so a single reading was not enough. This records the whole 5 second
// curve instead: type 's' to pause the live feed, 'g' to record a trial,
// 'c' to go back to live.

bool liveMode = true;
bool recording = false;
unsigned long startTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Type: s=stop live, g=go record trial, c=continue live");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 's') {
      liveMode = false;
      Serial.println(">> LIVE PAUSED");
    } else if (c == 'c') {
      liveMode = true;
      Serial.println(">> LIVE RESUMED");
    } else if (c == 'g') {
      recording = true;
      liveMode = false;
      startTime = millis();
      Serial.println("====== TRIAL START ======");
      Serial.println("Time(ms),Reading");
    }
  }

  long total = 0;
  for (int i = 0; i < 10; i++) total += analogRead(A0);
  int reading = total / 10;

  if (recording) {
    unsigned long elapsed = millis() - startTime;
    Serial.print(elapsed);
    Serial.print(",");
    Serial.println(reading);
    if (elapsed > 5000) {
      recording = false;
      Serial.println("====== TRIAL END ======");
    }
    delay(100);
  } else if (liveMode) {
    Serial.print("live: ");
    Serial.println(reading);
    delay(100);
  }
}
