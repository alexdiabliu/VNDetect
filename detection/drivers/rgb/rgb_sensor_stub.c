/**
 * Stub RGB photosensor: synthesizes a calm green-ish frame so host builds run.
 * Swap for a UART-framed driver that fills RgbSample the same way.
 */
#include "ports/rgb_sensor_port.h"

#include <string.h>

static RgbSample g_latest;

static int stub_init(void)
{
    memset(&g_latest, 0, sizeof(g_latest));
    g_latest.r = 10;
    g_latest.g = 200;
    g_latest.b = 10;
    g_latest.valid = true;
    g_latest.last_update_ms = 0;
    return 0;
}

static void stub_poll(uint32_t now_ms)
{
    /* Keep "alive" for timeout logic without real UART traffic. */
    g_latest.valid = true;
    g_latest.last_update_ms = now_ms;
}

static int stub_read(RgbSample *out)
{
    if (!out) {
        return -1;
    }
    *out = g_latest;
    return 0;
}

static const rgb_sensor_api_t g_api = {
    .init = stub_init,
    .poll = stub_poll,
    .read = stub_read,
};

const rgb_sensor_api_t *rgb_sensor_stub_api(void)
{
    return &g_api;
}
