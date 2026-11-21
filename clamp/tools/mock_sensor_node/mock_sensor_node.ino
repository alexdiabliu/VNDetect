/*
  VND Capstone - Workstream D test tool: Mock sensor node
  Board: any ESP32 with BLE (e.g. the spare XIAO ESP32C3)

  Pretends to be the sensor node's HM-10 so the clamp node can be developed
  before Workstream C's LaunchPad + HM-10 are ready. It uses the same BLE
  service/characteristic as an HM-10 (FFE0 / FFE1), so the clamp node code
  works unchanged against the real module later.

  Serial Monitor (115200) commands:
    a = send ALARM (repeats every 500 ms until the clamp node replies ACK)
    f = toggle fault mode (sends FAULT once a second instead of HB)
    h = toggle heartbeats on/off (off = simulate a dead/frozen sensor node)
    n = back to normal (stop alarm/fault, heartbeats on)
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

static BLEUUID SERVICE_UUID("0000ffe0-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_UUID   ("0000ffe1-0000-1000-8000-00805f9b34fb");

BLECharacteristic* ch = nullptr;
volatile bool clientConnected = false;
volatile bool ackReceived = false;

bool heartbeats = true;
bool alarmActive = false;
bool faultMode = false;
unsigned long lastHb = 0, lastAlarmTx = 0, alarmStart = 0;

void send(const char* msg) {
  if (!clientConnected) return;
  ch->setValue((uint8_t*)msg, strlen(msg));
  ch->notify();
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    clientConnected = true;
    Serial.println("clamp node connected");
  }
  void onDisconnect(BLEServer* s) override {
    clientConnected = false;
    Serial.println("clamp node disconnected - advertising again");
    s->getAdvertising()->start();
  }
};

class CharCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    String v = String(c->getValue().c_str());
    if (v.indexOf("ACK") >= 0) ackReceived = true;
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);

  BLEDevice::init("MOCK-SENSOR");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* svc = server->createService(SERVICE_UUID);
  ch = svc->createCharacteristic(
      CHAR_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  ch->addDescriptor(new BLE2902());
  ch->setCallbacks(new CharCallbacks());
  svc->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("Mock sensor node advertising as MOCK-SENSOR (FFE0/FFE1)");
  Serial.println("Commands: a=ALARM  f=toggle FAULT  h=toggle heartbeat  n=normal");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'a') { alarmActive = true; ackReceived = false; alarmStart = millis(); lastAlarmTx = 0;
                    Serial.println(">> ALARM"); }
    else if (c == 'f') { faultMode = !faultMode; Serial.printf("fault mode %s\n", faultMode ? "ON" : "OFF"); }
    else if (c == 'h') { heartbeats = !heartbeats; Serial.printf("heartbeats %s\n", heartbeats ? "ON" : "OFF"); }
    else if (c == 'n') { alarmActive = false; faultMode = false; heartbeats = true; Serial.println("normal"); }
  }

  if (heartbeats && !alarmActive && millis() - lastHb >= 1000) {
    lastHb = millis();
    send(faultMode ? "FAULT\n" : "HB\n");
  }

  if (alarmActive && !ackReceived && millis() - lastAlarmTx >= 500) {
    lastAlarmTx = millis();
    send("ALARM\n");
  }

  if (alarmActive && ackReceived) {
    Serial.printf("ACK received %lu ms after first ALARM\n", millis() - alarmStart);
    alarmActive = false;
    ackReceived = false;
  }

  delay(10);
}
