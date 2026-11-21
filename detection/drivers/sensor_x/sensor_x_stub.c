/**
 * Stub Sensor X (HM-10 path): emits a calm periodic value.
 * Real backend would parse HM-10 UART frames into SensorXSample.
 */
#include "ports/sensor_x_port.h"

#include <string.h>

static SensorXSample g_latest;

static int stub_init(void)
{
    memset(&g_latest, 0, sizeof(g_latest));
    g_latest.value = 0;
    g_latest.valid = true;
    g_latest.last_update_ms = 0;
    return 0;
}

static void stub_poll(uint32_t now_ms)
{
    g_latest.valid = true;
    g_latest.last_update_ms = now_ms;
    /* Placeholder "random-ish" but deterministic for host demos. */
    g_latest.value = (int32_t)((now_ms / 100U) % 50U);
}

static int stub_read(SensorXSample *out)
{
    if (!out) {
        return -1;
    }
    *out = g_latest;
    return 0;
}

static const sensor_x_api_t g_api = {
    .init = stub_init,
    .poll = stub_poll,
    .read = stub_read,
};

const sensor_x_api_t *sensor_x_stub_api(void)
{
    return &g_api;
}
