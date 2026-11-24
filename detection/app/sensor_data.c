
#include "sensor_data.h"

#define SENSOR_TIMEOUT_MS   2000U   // Flag FAULT if a sensor goes silent > 2s
#define RED_BLOOD_THRESHOLD 180U    // TEMPORARY TEST VALUE
#define SENSOR_X_THRESHOLD  100     // TEMPORARY TEST VALUE

SensorData process_sensor_data(RgbReading rgb, SensorXReading sensor_x, uint32_t now_ms) {
    SensorData out = {0, 0, true, 0};

    // 1. Health & timeout check (catches unplugged wires or frozen streams);
    bool rgb_alive = rgb.valid;
    bool sx_alive  = sensor_x.valid && ((now_ms - sensor_x.last_update_ms) <= SENSOR_TIMEOUT_MS);

    if (!rgb_alive || !sx_alive) {
        out.allSensorsOperational = false;
        return out;
    }

    // 2. Convert raw readings into unified metrics
    out.blood_reading = (int)rgb.r;
    out.distance_reading = sensor_x.value;

    // 3. Count independent threshold crossings
    if (out.blood_reading >= RED_BLOOD_THRESHOLD) {
        out.disturbance_count++;
    }
    if (out.distance_reading >= SENSOR_X_THRESHOLD) {
        out.disturbance_count++;
    }

    return out;
}