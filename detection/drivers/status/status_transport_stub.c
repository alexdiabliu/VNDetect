/**
 * Stub status transport: records last code for host inspection / logging.
 * Real backend writes status_to_wire(code) over UART/BT bridge to ESP32.
 */
#include "ports/status_transport_port.h"
#include "ports/uart_port.h"
#include "config/board_config.h"

#include <stdio.h>

static StatusCode g_last = STATUS_HEARTBEAT;

StatusCode status_transport_stub_last(void)
{
    return g_last;
}

static int stub_init(void)
{
    const uart_api_t *uart = uart_api();
    if (uart && uart->open) {
        (void)uart->open(CFG_UART_ESP32_ID, CFG_UART_BAUD_PLACEHOLDER);
    }
    g_last = STATUS_HEARTBEAT;
    return 0;
}

static int stub_send(StatusCode code)
{
    uint8_t wire = status_to_wire(code);
    const uart_api_t *uart = uart_api();

    g_last = code;
    if (uart && uart->write) {
        (void)uart->write(CFG_UART_ESP32_ID, &wire, 1);
    }

#if defined(MSP_HOST_BUILD)
    printf("[status] %s (wire=0x%02x)\n", status_name(code), (unsigned)wire);
#endif
    return 0;
}

static const status_transport_api_t g_api = {
    .init = stub_init,
    .send = stub_send,
};

const status_transport_api_t *status_transport_stub_api(void)
{
    return &g_api;
}
