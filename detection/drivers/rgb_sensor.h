

#ifndef RGB_SENSOR_H
#define RGB_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    bool valid;
} RgbReading;

void rgb_sensor_init(void);
void rgb_sensor_poll(void);
RgbReading rgb_sensor_get_latest(void);

#endif