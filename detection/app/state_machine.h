

#ifndef STATE_MACHINE_H // this header file will only run 'if not defined' previously
#define STATE_MACHINE_H

#include <stdint.h>
#include "sensor_data.h"

#define TIME_THRESHOLD 3000U

typedef enum {
    STATE_NORMAL = 0,
    STATE_SUSPECT,
    STATE_ALARM,
    STATE_FAULT
} System_state;

// Initializes functions
void state_machine_reset(void);
System_state update_state_machine(System_state current_state, SensorData sensors, uint32_t counterMs);

#endif