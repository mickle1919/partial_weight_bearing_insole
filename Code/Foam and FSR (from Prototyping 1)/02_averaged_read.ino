// Prototyping 1 - improved sketch. The raw readings were jumpy, so this
// takes 20 readings in quick succession and averages them, which cancels
// out the random fluctuation.

void setup() {
  Serial.begin(9600);
}

void loop() {
  long total = 0;
  int numReadings = 20;

  for (int i = 0; i < numReadings; i++) {
    total += analogRead(A0);
    delay(2);
  }

  int average = total / numReadings;
  Serial.println(average);
  delay(100);
}
