/**
 * Sensor X input via HM-10 BLE → UART bridge.
 */
#ifndef SENSOR_X_PORT_H
#define SENSOR_X_PORT_H

#include <stdint.h>
#include "protocol/types.h"

typedef struct sensor_x_api {
    int (*init)(void);
    void (*poll)(uint32_t now_ms);
    int (*read)(SensorXSample *out);
} sensor_x_api_t;

void sensor_x_bind(const sensor_x_api_t *api);
const sensor_x_api_t *sensor_x_api(void);

int sensor_x_init(void);
void sensor_x_poll(uint32_t now_ms);
int sensor_x_read(SensorXSample *out);

#endif /* SENSOR_X_PORT_H */
