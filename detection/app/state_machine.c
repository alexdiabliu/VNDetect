
#include "state_machine.h"

static uint32_t suspect_start = 0;

void state_machine_reset(void) {
    suspect_start = 0;
}

System_state update_state_machine(System_state current_state, SensorData sensors, uint32_t counterMs) {
    /* 1. Keep active ALARM latched regardless of subsequent sensor disconnects */
    if (current_state == STATE_ALARM) {
        return STATE_ALARM;
    }

    /* 2. Hardware health check */
    if (!sensors.allSensorsOperational) {
        return STATE_FAULT;
    }

    switch (current_state) {
        case STATE_NORMAL:
            if (sensors.disturbance_count >= 2) {
                return STATE_ALARM;
            } else if (sensors.disturbance_count == 1) {
                suspect_start = counterMs;
                return STATE_SUSPECT;
            }
            return STATE_NORMAL;

        case STATE_SUSPECT:
            if (sensors.disturbance_count >= 2 || (counterMs - suspect_start) >= TIME_THRESHOLD) {
                return STATE_ALARM;
            } else if (sensors.disturbance_count == 0) {
                return STATE_NORMAL;
            }
            return STATE_SUSPECT;

        case STATE_FAULT:
        default:
            return STATE_FAULT;
    }
}