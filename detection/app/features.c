#include "features.h"
#include "config/board_config.h"

SensorFeatures features_from_samples(const RgbSample *rgb,
                                     const SensorXSample *sensor_x,
                                     uint32_t now_ms)
{
    SensorFeatures out;
    bool rgb_alive;
    bool sx_alive;

    out.blood_reading = 0;
    out.distance_reading = 0;
    out.disturbance_count = 0;
    out.rgb_healthy = false;
    out.sensor_x_healthy = false;
    out.all_sensors_operational = false;

    if (!rgb || !sensor_x) {
        return out;
    }

    rgb_alive = rgb->valid &&
                ((now_ms - rgb->last_update_ms) <= CFG_SENSOR_TIMEOUT_MS);
    sx_alive = sensor_x->valid &&
               ((now_ms - sensor_x->last_update_ms) <= CFG_SENSOR_TIMEOUT_MS);

    out.rgb_healthy = rgb_alive;
    out.sensor_x_healthy = sx_alive;
    out.all_sensors_operational = rgb_alive && sx_alive;

    if (!out.all_sensors_operational) {
        return out;
    }

    out.blood_reading = (int)rgb->r;
    out.distance_reading = (int)sensor_x->value;

    if (out.blood_reading >= (int)CFG_RGB_RED_THRESHOLD) {
        out.disturbance_count++;
    }
    if (out.distance_reading >= CFG_SENSOR_X_THRESHOLD) {
        out.disturbance_count++;
    }

    return out;
}
