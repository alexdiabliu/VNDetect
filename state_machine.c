
#include <stdint.h>
#include <stdbool.h>

const int DISTANCE_THRESHOLD = 100;
const int BLOOD_THRESHOLD = 50;
const int TIME_THRESHOLD = 3000;

typedef enum {
    STATE_NORMAL,
    STATE_SUSPECT,
    STATE_ALARM,
    STATE_FAULT

} System_state;


typedef struct {
    int distance_reading;
    int blood_reading;
    bool allSensorsOperational;
    int disturbance_count;
} SensorData;

System_state update_state_machine(System_state current_state, SensorData sensors, uint32_t counterMs) {
    static uint32_t suspect_start = 0;
    // Hardware check first (Fault)
    if (!sensors.allSensorsOperational) {
        return STATE_FAULT;
    }

    switch (current_state) {
        case STATE_NORMAL:
            if (sensors.disturbance_count >= 2) {
                return STATE_ALARM;
            }
            else if (sensors.disturbance_count == 1) {
                suspect_start = counterMs;
                return STATE_SUSPECT;
            }
            return STATE_NORMAL;
        case STATE_SUSPECT:
            if (sensors.disturbance_count >= 2 || (counterMs-suspect_start) >= TIME_THRESHOLD) {
                return STATE_ALARM;
            }
            else if (sensors.disturbance_count == 0) {
                return STATE_NORMAL;
            }
            return STATE_SUSPECT;
        case STATE_ALARM:
            return STATE_ALARM;
        case STATE_FAULT:
        default:
            return STATE_FAULT;


    }
}