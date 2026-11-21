/**
 * Thin runtime registration for active driver vtables.
 */
#include "uart_port.h"
#include "rgb_sensor_port.h"
#include "sensor_x_port.h"
#include "status_transport_port.h"

#include <stddef.h>

static const uart_api_t *g_uart;
static const rgb_sensor_api_t *g_rgb;
static const sensor_x_api_t *g_sensor_x;
static const status_transport_api_t *g_status;

void uart_bind(const uart_api_t *api) { g_uart = api; }
const uart_api_t *uart_api(void) { return g_uart; }

void rgb_sensor_bind(const rgb_sensor_api_t *api) { g_rgb = api; }
const rgb_sensor_api_t *rgb_sensor_api(void) { return g_rgb; }

int rgb_sensor_init(void)
{
    return (g_rgb && g_rgb->init) ? g_rgb->init() : -1;
}

void rgb_sensor_poll(uint32_t now_ms)
{
    if (g_rgb && g_rgb->poll) {
        g_rgb->poll(now_ms);
    }
}

int rgb_sensor_read(RgbSample *out)
{
    return (g_rgb && g_rgb->read) ? g_rgb->read(out) : -1;
}

void sensor_x_bind(const sensor_x_api_t *api) { g_sensor_x = api; }
const sensor_x_api_t *sensor_x_api(void) { return g_sensor_x; }

int sensor_x_init(void)
{
    return (g_sensor_x && g_sensor_x->init) ? g_sensor_x->init() : -1;
}

void sensor_x_poll(uint32_t now_ms)
{
    if (g_sensor_x && g_sensor_x->poll) {
        g_sensor_x->poll(now_ms);
    }
}

int sensor_x_read(SensorXSample *out)
{
    return (g_sensor_x && g_sensor_x->read) ? g_sensor_x->read(out) : -1;
}

void status_transport_bind(const status_transport_api_t *api) { g_status = api; }
const status_transport_api_t *status_transport_api(void) { return g_status; }

int status_transport_init(void)
{
    return (g_status && g_status->init) ? g_status->init() : -1;
}

int status_transport_send(StatusCode code)
{
    return (g_status && g_status->send) ? g_status->send(code) : -1;
}
