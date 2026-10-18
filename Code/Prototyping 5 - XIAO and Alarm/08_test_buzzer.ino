#include "Adafruit_TinyUSB.h"

#define BUZZ      D6
#define PIP_MS    120     // length of each pip
#define GAP_MS    100     // silence between the two pips
#define CYCLE_MS  1200    // silence before the pattern repeats

unsigned long beeps = 0;

void pip(int n) {
  digitalWrite(BUZZ, HIGH);
  Serial.print("  BEEP ");
  Serial.print(n);
  Serial.print("  (pin HIGH, ");
  Serial.print(PIP_MS);
  Serial.println(" ms)");
  delay(PIP_MS);

  digitalWrite(BUZZ, LOW);
  Serial.println("  ---- off");
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW);

  while (!Serial && millis() < 3000) { }

  Serial.println("=== BUZZER TEST ===");
  Serial.println("Pin D6, two pips then a pause, repeating.");
  Serial.println("If the serial log says BEEP and you hear nothing:");
  Serial.println("  - the buzzer may be in backwards (long lead to the resistor)");
  Serial.println("  - the resistor may be too large - step down");
  Serial.println("  - it may be the PASSIVE buzzer, which cannot work here");
  Serial.println("  - a flying lead may not be seated in the perfboard");
  Serial.println();
}

void loop() {
  beeps++;
  Serial.print("cycle ");
  Serial.print(beeps);
  Serial.print("  @ ");
  Serial.print(millis() / 1000.0, 1);
  Serial.println(" s");

  pip(1);
  delay(GAP_MS);
  pip(2);

  Serial.println();
  delay(CYCLE_MS);
}
