/*
  VND Capstone - Workstream D: Clamp node (stepper OR servo)
  Board: Seeed XIAO ESP32C3
  Library: ESP32Servo (Library Manager) - only needed when USE_STEPPER is 0.

  ACTUATOR SWITCH: set USE_STEPPER below.
    1 = 28BYJ-48 stepper + ULN2003 driver board (stopgap)
    0 = 180-degree servo (MG90S) once it arrives

  What it does
  - Connects as a BLE client to the sensor node's HM-10 (service FFE0, characteristic FFE1).
  - Reads newline-terminated text messages: HB, ALARM, FAULT.
  - HB     -> heartbeat, ~1/s while the sensor node is normal.
  - ALARM  -> closes the clamp (servo), turns the buzzer on, replies "ACK\n".
  - FAULT  -> sent ~1/s instead of HB while the sensor node has a fault: beeps, does NOT clamp.
  - No HB for 3 s after a connection (or link drops) -> fail-safe local alarm (beeping), no clamp.
  - Serial Monitor (115200) commands for bench testing:
      c = clamp now, o = open (reset), b = buzzer test, s = print status

  Wiring (see README.md)
  - STEPPER: ULN2003 IN1 -> D2, IN2 -> D4, IN3 -> D5, IN4 -> D10
             ULN2003 + -> XIAO 5V pin (USB power), ULN2003 - -> GND. Motor plugs into the white socket.
             (D0, D8, D9 are boot-strapping pins on the C3 - leave them free.)
  - SERVO:   signal -> D2, power -> 4xAA pack (+), 470 uF across servo +/-
  - Buzzer MOSFET gate (IRLB8721) -> D3 through 100 ohm, 100k gate-to-GND pull-down
  - Reset button (optional) -> D1 to GND (internal pull-up)
  - ALL grounds common: XIAO GND, 4xAA (-), MOSFET source, servo brown wire
*/

#define USE_STEPPER 0   // 1 = 28BYJ-48 stepper, 0 = 180-degree servo

// HM-10 LOOPBACK TEST: lets ONE XIAO test the real Bluetooth link.
// The XIAO sends HB/ALARM to the HM-10 over UART (D6 -> HM-10 RXD); the HM-10 transmits them
// over BLE; the XIAO's BLE client receives them as if from the sensor node. The ACK comes back
// over BLE -> HM-10 TXD -> XIAO D7, giving a real round-trip time.
#define HM10_LOOPBACK_TEST 1

#include <BLEDevice.h>
#if !USE_STEPPER
#include <ESP32Servo.h>
#endif

// ---------- Pins ----------
const int PIN_SERVO  = D2;
const int PIN_BUZZER = D3;   // drives the IRLB8721 gate (HIGH = buzzer on)
const int PIN_RESET  = D1;   // optional push button to GND

// ---------- Tunables (adjust on the bench) ----------
const int  SERVO_OPEN_DEG   = 0;     // clamp released
const int  SERVO_CLAMP_DEG  = 90;    // clamp closed - tune against the mock tubing

// Stepper (28BYJ-48, full-step two-phase drive: ~2048 steps per output revolution)
const int  STEP_PINS[4]     = {D2, D4, D5, D10};   // ULN2003 IN1..IN4
long       CLAMP_STEPS      = 512;    // steps from "open" to "clamped" (512 = ~90 deg). Negative = other direction.
const unsigned long STEP_INTERVAL_US = 2500;   // time per step; smaller = faster but weaker (min ~2000)
const bool RELEASE_COILS_WHEN_OPEN = true;     // cut coil current when open (saves power, motor stays cool)
const long JOG_STEPS        = 32;     // serial j / k jog size while tuning
const unsigned long HB_TIMEOUT_MS   = 3000;  // fail-safe if no heartbeat for this long
const unsigned long SCAN_SECONDS    = 5;     // BLE scan window per attempt

// ---------- BLE identifiers (HM-10 defaults) ----------
static BLEUUID SERVICE_UUID("0000ffe0-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_UUID   ("0000ffe1-0000-1000-8000-00805f9b34fb");
// Fallback names if the module does not advertise FFE0 (HM-10 default is "HMSoft").
const char* KNOWN_NAMES[] = {"HMSoft", "HM-10", "BT05", "MOCK-SENSOR"};

// ---------- State ----------
enum NodeState { WAITING, NORMAL, ALARMED, FAULTED, LINK_LOST };
volatile NodeState state = WAITING;

#if !USE_STEPPER
Servo clampServo;
#endif

// Stepper state. Position 0 = "open". Whatever position the motor is in at power-up
// is taken as open, so set the clamp to open by hand before powering on (or jog + 'z').
long stepPos = 0, stepTarget = 0;
int  stepPhase = 0;
bool coilsOn = false;
unsigned long lastStepUs = 0, moveStartMs = 0;
const uint8_t STEP_SEQ[4][4] = {{1,1,0,0},{0,1,1,0},{0,0,1,1},{1,0,0,1}};
BLEAdvertisedDevice* target = nullptr;
BLEClient* client = nullptr;
BLERemoteCharacteristic* remoteChar = nullptr;

volatile bool doConnect = false;
volatile bool scanning = false;
volatile bool connected = false;
bool everConnected = false;
unsigned long lastHbMs = 0;

// Incoming bytes go into a small ring buffer from the BLE callback and are parsed in loop().
const int RX_SIZE = 256;
volatile char rxRing[RX_SIZE];
volatile int rxHead = 0, rxTail = 0;
portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;
String pending;

// ---------- Helpers ----------
const char* stateName(NodeState s) {
  switch (s) {
    case WAITING:   return "WAITING";
    case NORMAL:    return "NORMAL";
    case ALARMED:   return "ALARMED";
    case FAULTED:   return "FAULTED";
    case LINK_LOST: return "LINK_LOST";
  }
  return "?";
}

void logEvent(const char* msg) {
  Serial.printf("[%8lu ms] %-9s %s\n", millis(), stateName(state), msg);
}

void setState(NodeState s) {
  if (state != s) {
    state = s;
    logEvent("state change");
  }
}

void coilsWrite(int phase) {
  for (int i = 0; i < 4; i++) digitalWrite(STEP_PINS[i], STEP_SEQ[phase][i]);
  coilsOn = true;
}

void coilsOff() {
  for (int i = 0; i < 4; i++) digitalWrite(STEP_PINS[i], LOW);
  coilsOn = false;
}

// Non-blocking: called every loop pass, moves at most one step per STEP_INTERVAL_US,
// so Bluetooth, heartbeat checks and the buzzer keep running while the clamp moves.
void stepperService() {
#if USE_STEPPER
  if (stepPos == stepTarget) {
    if (coilsOn && stepTarget == 0 && RELEASE_COILS_WHEN_OPEN) coilsOff();
    return;
  }
  if (micros() - lastStepUs < STEP_INTERVAL_US) return;
  lastStepUs = micros();
  int dir = (stepTarget > stepPos) ? 1 : -1;
  stepPhase = (stepPhase + dir + 4) % 4;
  coilsWrite(stepPhase);
  stepPos += dir;
  if (stepPos == stepTarget) {
    Serial.printf("[%8lu ms] clamp reached step %ld in %lu ms\n",
                  millis(), stepPos, millis() - moveStartMs);
  }
#endif
}

void clampClose() {
#if USE_STEPPER
  stepTarget = CLAMP_STEPS;
  moveStartMs = millis();
#else
  clampServo.write(SERVO_CLAMP_DEG);
#endif
  logEvent("CLAMP CLOSED (commanded)");
}

void clampOpen() {
#if USE_STEPPER
  stepTarget = 0;
  moveStartMs = millis();
#else
  clampServo.write(SERVO_OPEN_DEG);
#endif
  logEvent("clamp opened (commanded)");
}

void sendToSensor(const char* msg) {
  if (!connected || !remoteChar) return;
  // HM-10's FFE1 accepts write-without-response; fall back to a normal write otherwise.
  bool withResponse = !remoteChar->canWriteNoResponse();
  remoteChar->writeValue((uint8_t*)msg, strlen(msg), withResponse);
}

// ---------- Message handling ----------
void handleMessage(const String& msg) {
  if (msg == "HB") {
    lastHbMs = millis();
    if (state == WAITING || state == LINK_LOST || state == FAULTED) setState(NORMAL);
  } else if (msg == "ALARM") {
    unsigned long t0 = millis();
    if (state != ALARMED) {
      clampClose();
      digitalWrite(PIN_BUZZER, HIGH);
      setState(ALARMED);
    }
    sendToSensor("ACK\n");
    Serial.printf("[%8lu ms] ALARM handled in %lu ms (receive -> clamp command + ACK)\n",
                  millis(), millis() - t0);
  } else if (msg == "FAULT") {
    // The sensor node sends FAULT about once a second instead of HB while a fault lasts,
    // so it also proves the link is alive. HB resuming means the fault cleared.
    lastHbMs = millis();
    if (state != ALARMED) setState(FAULTED);   // never downgrade an alarm
  } else if (msg.length() > 0) {
    Serial.printf("[%8lu ms] unknown message: '%s'\n", millis(), msg.c_str());
  }
}

void processRx() {
  while (true) {
    char c;
    portENTER_CRITICAL(&rxMux);
    bool empty = (rxTail == rxHead);
    if (!empty) { c = rxRing[rxTail]; rxTail = (rxTail + 1) % RX_SIZE; }
    portEXIT_CRITICAL(&rxMux);
    if (empty) break;
    pending += c;
  }
  int nl;
  while ((nl = pending.indexOf('\n')) >= 0) {
    String line = pending.substring(0, nl);
    pending.remove(0, nl + 1);
    line.trim();                      // strips \r and spaces
    handleMessage(line);
  }
  if (pending.length() > 64) pending = "";   // guard against garbage with no newline
}

// ---------- HM-10 loopback test (stands in for the sensor node's UART) ----------
#if HM10_LOOPBACK_TEST
bool lbHeartbeat = true, lbFault = false, lbAlarm = false;
unsigned long lbLastHb = 0, lbLastAlarm = 0, lbAlarmStart = 0;
String lbRx;

void loopbackService() {
  if (!connected) return;   // HM-10 only forwards UART data while a BLE client is connected
  if (lbHeartbeat && !lbAlarm && millis() - lbLastHb >= 1000) {
    lbLastHb = millis();
    Serial1.print(lbFault ? "FAULT\n" : "HB\n");
  }
  if (lbAlarm && millis() - lbLastAlarm >= 500) {     // repeat ALARM until ACK, like the real sensor node
    lbLastAlarm = millis();
    Serial1.print("ALARM\n");
  }
  while (Serial1.available()) {                       // whatever the clamp node wrote over BLE
    char c = (char)Serial1.read();
    if (c == '\n' || c == '\r') {
      lbRx.trim();
      if (lbRx.indexOf("ACK") >= 0 && lbAlarm) {
        Serial.printf("[%8lu ms] LOOPBACK: ACK came back %lu ms after first ALARM\n",
                      millis(), millis() - lbAlarmStart);
        lbAlarm = false;
      } else if (lbRx.length() > 0) {
        Serial.printf("[%8lu ms] HM-10 UART says: %s\n", millis(), lbRx.c_str());
      }
      lbRx = "";
    } else if (lbRx.length() < 64) {
      lbRx += c;
    }
  }
}
#endif

// ---------- BLE callbacks ----------
void scanDone(BLEScanResults) {
  BLEDevice::getScan()->clearResults();
  scanning = false;
}

void notifyCallback(BLERemoteCharacteristic*, uint8_t* data, size_t len, bool) {
  portENTER_CRITICAL(&rxMux);
  for (size_t i = 0; i < len; i++) {
    int next = (rxHead + 1) % RX_SIZE;
    if (next == rxTail) break;          // buffer full: drop the rest
    rxRing[rxHead] = (char)data[i];
    rxHead = next;
  }
  portEXIT_CRITICAL(&rxMux);
}

class ClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient*) override {}
  void onDisconnect(BLEClient*) override {
    connected = false;
  }
};

class ScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) override {
    bool match = dev.haveServiceUUID() && dev.isAdvertisingService(SERVICE_UUID);
    if (!match && dev.haveName()) {
      for (const char* n : KNOWN_NAMES) {
        if (strcmp(dev.getName().c_str(), n) == 0) match = true;
      }
    }
    if (match) {
      BLEDevice::getScan()->stop();
      scanning = false;
      if (target) delete target;
      target = new BLEAdvertisedDevice(dev);
      doConnect = true;
    }
  }
};

bool connectToSensor() {
  Serial.printf("[%8lu ms] connecting to %s\n", millis(), target->getAddress().toString().c_str());
  if (!client) {
    client = BLEDevice::createClient();
    client->setClientCallbacks(new ClientCallbacks());
  }
  if (!client->connect(target)) return false;

  BLERemoteService* svc = client->getService(SERVICE_UUID);
  if (!svc) { client->disconnect(); return false; }
  remoteChar = svc->getCharacteristic(CHAR_UUID);
  if (!remoteChar) { client->disconnect(); return false; }
  if (remoteChar->canNotify()) remoteChar->registerForNotify(notifyCallback);

  connected = true;
  everConnected = true;
  lastHbMs = millis();      // give the sensor node a full timeout window to start
  logEvent("connected to sensor node");
  return true;
}

// ---------- Setup / loop ----------
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  pinMode(PIN_RESET, INPUT_PULLUP);

#if USE_STEPPER
  for (int i = 0; i < 4; i++) pinMode(STEP_PINS[i], OUTPUT);
  coilsOff();
  Serial.println("Stepper mode. Tuning: j/k = jog +/-, p = print position, z = set current position as open");
#else
  ESP32PWM::allocateTimer(0);
  clampServo.setPeriodHertz(50);
  clampServo.attach(PIN_SERVO, 500, 2400);
  clampOpen();
#endif

#if HM10_LOOPBACK_TEST
  Serial1.begin(9600, SERIAL_8N1, D7, D6);   // RX = D7 (from HM-10 TXD), TX = D6 (to HM-10 RXD)
  Serial.println("HM-10 loopback test ON. A = send ALARM, F = toggle FAULT, H = toggle heartbeat, N = normal");
#endif

  BLEDevice::init("CLAMP-NODE");
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);

  logEvent("clamp node ready - scanning for sensor node");
}

void loop() {
  // 1. Connect / reconnect
  if (doConnect) {
    doConnect = false;
    if (!connectToSensor()) logEvent("connect failed - will rescan");
  }
  if (!connected) {
    if (everConnected && state != ALARMED && state != LINK_LOST) {
      setState(LINK_LOST);
      logEvent("LINK LOST (disconnected)");
    }
    if (!scanning && !doConnect) {
      scanning = true;
      BLEDevice::getScan()->start(SCAN_SECONDS, scanDone, false);   // non-blocking scan
    }
  }

  // 2. Parse any received messages
  processRx();

  // 3. Heartbeat fail-safe
  if (connected && (state == NORMAL || state == FAULTED) && millis() - lastHbMs > HB_TIMEOUT_MS) {
    setState(LINK_LOST);
    logEvent("LINK LOST (no heartbeat)");
  }

  // 4. Buzzer patterns (ALARMED = solid, set when the alarm fires)
  if (state == FAULTED || state == LINK_LOST) {
    // FAULT: short beeps; LINK_LOST: slower beeps - so you can tell them apart by ear
    unsigned long period = (state == FAULTED) ? 400 : 1000;
    digitalWrite(PIN_BUZZER, (millis() % period) < 150 ? HIGH : LOW);
  } else if (state != ALARMED) {
    digitalWrite(PIN_BUZZER, LOW);
  }

  // 5. Manual reset (button or Serial 'o'): opens clamp, silences buzzer
  bool resetPressed = digitalRead(PIN_RESET) == LOW;
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') resetPressed = true;
    else if (c == 'c') { clampClose(); }
    else if (c == 'b') { digitalWrite(PIN_BUZZER, HIGH); delay(300); digitalWrite(PIN_BUZZER, LOW); }
#if USE_STEPPER
    else if (c == 'j') { stepTarget += JOG_STEPS; moveStartMs = millis(); }
    else if (c == 'k') { stepTarget -= JOG_STEPS; moveStartMs = millis(); }
    else if (c == 'p') { Serial.printf("stepper position = %ld (CLAMP_STEPS = %ld)\n", stepPos, CLAMP_STEPS); }
    else if (c == 'z') { stepPos = stepTarget = 0; coilsOff(); Serial.println("current position set as OPEN (0)"); }
#endif
#if HM10_LOOPBACK_TEST
    else if (c == 'A') { lbAlarm = true; lbAlarmStart = millis(); lbLastAlarm = 0; Serial.println(">> loopback: sending ALARM"); }
    else if (c == 'F') { lbFault = !lbFault; Serial.printf(">> loopback: fault mode %s\n", lbFault ? "ON" : "OFF"); }
    else if (c == 'H') { lbHeartbeat = !lbHeartbeat; Serial.printf(">> loopback: heartbeats %s\n", lbHeartbeat ? "ON" : "OFF"); }
    else if (c == 'N') { lbAlarm = false; lbFault = false; lbHeartbeat = true; Serial.println(">> loopback: normal"); }
#endif
    else if (c == 's') {
      Serial.printf("state=%s connected=%d lastHB=%lu ms ago\n",
                    stateName(state), connected, millis() - lastHbMs);
    }
  }
  if (resetPressed) {
    clampOpen();
    digitalWrite(PIN_BUZZER, LOW);
    lastHbMs = millis();
    setState(connected ? NORMAL : WAITING);
    logEvent("manual reset");
    delay(300);   // crude debounce
  }

  // 6. Move the stepper (non-blocking)
  stepperService();

#if HM10_LOOPBACK_TEST
  // 7. Feed the HM-10 like the sensor node would
  loopbackService();
#endif

  delay(1);   // short yield so the BLE stack runs; stepper timing uses micros()
}
