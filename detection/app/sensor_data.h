
#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/rgb_sensor.h"
#include "../drivers/ble_link.h"

typedef struct {
    int distance_reading;
    int blood_reading;
    bool allSensorsOperational;
    int disturbance_count;
} SensorData;

SensorData process_sensor_data(RgbReading rgb, SensorXReading sensor_x, uint32_t now_ms);

#endif