// Prototyping 1 - first sketch, conductive foam / sensor sheet testing.
//
// Reads the voltage divider on A0 and prints it. Nothing else.

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  delay(100);
}
