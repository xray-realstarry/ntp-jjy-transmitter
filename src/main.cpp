#include <Arduino.h>

#include "jjy_encoder.h"

// Placeholder: prints the frame for a fixed time to check the encoder on
// target. WiFi/NTP and carrier generation are not implemented yet.
void setup() {
  Serial.begin(115200);
  delay(1000);

  const jjy::DateTime t{2026, 10, 4, 12, 34, 0};
  jjy::Frame frame;
  if (!jjy::encode(t, jjy::LeapSecond::None, frame)) {
    Serial.println("encode failed");
    return;
  }
  for (jjy::Symbol s : frame) {
    Serial.print(s == jjy::Symbol::Marker ? 'M' : (s == jjy::Symbol::One ? '1' : '0'));
  }
  Serial.println();
}

void loop() {}
