/**
 * RGB photosensor input port (24-bit RGB via UART on real hardware).
 */
#ifndef RGB_SENSOR_PORT_H
#define RGB_SENSOR_PORT_H

#include <stdint.h>
#include "protocol/types.h"

typedef struct rgb_sensor_api {
    int (*init)(void);
    void (*poll)(uint32_t now_ms);
    int (*read)(RgbSample *out);
} rgb_sensor_api_t;

void rgb_sensor_bind(const rgb_sensor_api_t *api);
const rgb_sensor_api_t *rgb_sensor_api(void);

/* Convenience wrappers used by app code */
int rgb_sensor_init(void);
void rgb_sensor_poll(uint32_t now_ms);
int rgb_sensor_read(RgbSample *out);

#endif /* RGB_SENSOR_PORT_H */
