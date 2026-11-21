/**
 * Shared sample and feature types — no hardware includes.
 */
#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool valid;
    uint32_t last_update_ms;
} RgbSample;

typedef struct {
    int32_t value; /* opaque Sensor X payload until format is fixed */
    bool valid;
    uint32_t last_update_ms;
} SensorXSample;

typedef struct {
    bool rgb_healthy;
    bool sensor_x_healthy;
    bool all_sensors_operational;
    int disturbance_count; /* 0..2 independent threshold crossings */
    int blood_reading;     /* derived feature (placeholder: red channel) */
    int distance_reading;  /* derived feature (placeholder: Sensor X value) */
} SensorFeatures;

#endif /* PROTOCOL_TYPES_H */
