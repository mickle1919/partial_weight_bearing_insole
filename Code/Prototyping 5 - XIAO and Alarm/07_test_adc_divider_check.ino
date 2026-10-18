void setup() {
  Serial.begin(115200);

  // Make the XIAO behave like the Uno. Harmless on the Uno itself.
  #if defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_NRF52840)
    analogReadResolution(10);     // 0..1023
    analogReference(AR_VDD4);     // measure against the supply, so the
                                  // reading is a ratio and the battery
                                  // voltage does not matter
  #endif

  Serial.println("ADC transfer check — 100k / 100k divider on A0");
  Serial.println("Expect ~511 (half of full scale).");
}

void loop() {
  long total = 0;
  for (int i = 0; i < 20; i++) {
    total += analogRead(A0);
    delay(2);
  }
  Serial.println(total / 20);
  delay(200);
}
