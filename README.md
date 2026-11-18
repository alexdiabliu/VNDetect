# VNDetect

## State Machine
### 1. Normal
**Cause:** Distance and Optical Sensors readings are both within healthy bounds
**Effect:** Sends Heartbeats (HB) every second

### 2. Suspect
**Cause:** Sensor detects outlier in one sensor
**Effect:** Sends Heartbeats (HB) every second, monitors how long the anomaly lasts

### 3. Alarm
**Cause:** The Suspect state lasts longer than 3 seconds OR both sensors (Distance/Optical) detect a fail
**Effect:** Sends ALARM

### 4. Fault
**Cause:** 1+ sensors stop responding, time out, or returns invalid error values
**Effect:** Sends FAULT