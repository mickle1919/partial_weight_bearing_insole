// Prototyping 1, second device - Wheatstone bridge with the FSR as the
// fourth arm. Three 10 ohm resistors make the fixed arms. A0 is the fixed
// reference midpoint (~512), A1 is the FSR side.

void setup() {
  Serial.begin(9600);
}

void loop() {
  long totalA0 = 0;
  long totalA1 = 0;
  int numReadings = 20;

  for (int i = 0; i < numReadings; i++) {
    totalA0 += analogRead(A0);
    totalA1 += analogRead(A1);
    delay(2);
  }

  int a0 = totalA0 / numReadings;
  int a1 = totalA1 / numReadings;
  int difference = a0 - a1;

  Serial.print("A0: ");
  Serial.print(a0);
  Serial.print("  A1: ");
  Serial.print(a1);
  Serial.print("  Difference: ");
  Serial.println(difference);

  delay(100);
}
