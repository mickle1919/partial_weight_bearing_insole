#include "Adafruit_TinyUSB.h"

#define BUZZ         D6
#define BUZZ_ACTIVE  HIGH    // transistor conducts
#define BUZZ_IDLE    LOW     // transistor off

bool alarmMode = false;
bool buzzOn = false;
unsigned long buzzNext = 0;
unsigned long lastPrint = 0;

int readAvg(int pin) {
  long total = 0;
  for (int i = 0; i < 5; i++) { total += analogRead(pin); }
  return total / 5;
}

void buzzOff() {
  digitalWrite(BUZZ, BUZZ_IDLE);
  buzzOn = false;
  alarmMode = false;
}

void pipPip() {
  digitalWrite(BUZZ, BUZZ_ACTIVE); delay(80);
  digitalWrite(BUZZ, BUZZ_IDLE);   delay(220);
  digitalWrite(BUZZ, BUZZ_ACTIVE); delay(80);
  digitalWrite(BUZZ, BUZZ_IDLE);
}

void setup() {
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, BUZZ_IDLE);

  Serial.begin(115200);
  analogReadResolution(10);
  analogReference(AR_VDD4);

  while (!Serial && millis() < 3000) { }

  Serial.println("=== INTERACTIVE BUZZER TEST (transistor, non-inverted) ===");
  Serial.println("b = on   s = off   p = one pip-pip   a = alarm rhythm");
  Serial.println("Channel readings print once a second.");
  Serial.println();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'b') {
      alarmMode = false;
      digitalWrite(BUZZ, BUZZ_ACTIVE);
      buzzOn = true;
      Serial.println(">> BUZZER ON");
    }
    else if (c == 's') {
      buzzOff();
      Serial.println(">> BUZZER OFF");
    }
    else if (c == 'p') {
      Serial.println(">> pip-pip");
      pipPip();
    }
    else if (c == 'a') {
      alarmMode = true;
      buzzNext = millis();
      Serial.println(">> ALARM RHYTHM - press 's' to stop");
    }
  }

  if (alarmMode) {
    unsigned long now = millis();
    if (now >= buzzNext) {
      buzzOn = !buzzOn;
      digitalWrite(BUZZ, buzzOn ? BUZZ_ACTIVE : BUZZ_IDLE);
      buzzNext = now + (buzzOn ? 80 : 220);
    }
  }

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.print("heel D1: ");
    Serial.print(readAvg(D1));
    Serial.print("   ball D0: ");
    Serial.print(readAvg(D0));
    Serial.print("   buzzer: ");
    Serial.println(buzzOn ? "SOUNDING" : "silent");
  }
}
