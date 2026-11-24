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

void rgb_sensor_init(void); // clear last readings and resets RgbReading struct
void rgb_sensor_poll(void); // Read RGB input from log/live data
RgbReading rgb_sensor_get_latest(void); // return the struct to main.c

#endif