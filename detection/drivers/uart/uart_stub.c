/**
 * Stub UART: discards writes, never returns RX data.
 * Replace with a real USCI/eUSCI (or host serial) backend later.
 */
#include "ports/uart_port.h"

static int stub_open(int uart_id, uint32_t baud)
{
    (void)uart_id;
    (void)baud;
    return 0;
}

static int stub_read(int uart_id, uint8_t *buf, size_t max_len)
{
    (void)uart_id;
    (void)buf;
    (void)max_len;
    return 0;
}

static int stub_write(int uart_id, const uint8_t *buf, size_t len)
{
    (void)uart_id;
    (void)buf;
    return (int)len;
}

static void stub_close(int uart_id)
{
    (void)uart_id;
}

static const uart_api_t g_api = {
    .open = stub_open,
    .read = stub_read,
    .write = stub_write,
    .close = stub_close,
};

const uart_api_t *uart_stub_api(void)
{
    return &g_api;
}
