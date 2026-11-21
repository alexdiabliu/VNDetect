/*
  ULN2003 channel test - XIAO ESP32C3
  Turns IN1, IN2, IN3, IN4 on one at a time (1 s each), then all off (2 s), forever.
  Watch the 4 LEDs on the driver board and the Serial Monitor (115200).

  Expected: exactly ONE LED lit, moving A -> B -> C -> D, then all dark.
  - An LED that never lights        -> that IN wire is on the wrong pin, loose, or the channel is dead.
  - An LED that is ALWAYS lit       -> that IN wire is on a pin this sketch doesn't drive,
                                       or that driver channel is damaged (stuck on).
*/

const int STEP_PINS[4] = {D2, D4, D5, D10};   // IN1..IN4
const char* NAMES[4]   = {"IN1 (D2)", "IN2 (D4)", "IN3 (D5)", "IN4 (D10)"};

void allOff() {
  for (int i = 0; i < 4; i++) digitalWrite(STEP_PINS[i], LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) pinMode(STEP_PINS[i], OUTPUT);
  allOff();
}

void loop() {
  for (int i = 0; i < 4; i++) {
    allOff();
    digitalWrite(STEP_PINS[i], HIGH);
    Serial.printf("ON: %s  -> LED %c should be the only one lit\n", NAMES[i], 'A' + i);
    delay(1000);
  }
  allOff();
  Serial.println("ALL OFF -> every LED should be dark");
  delay(2000);
}
