// Prototyping 1 - first sketch, conductive foam / sensor sheet testing.
//
// NOTE: the original file was overwritten during development. This is
// transcribed from the screenshot on page 3 of the Prototyping 1 notebook.
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
