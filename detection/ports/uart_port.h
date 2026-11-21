/**
 * Thin UART byte-stream port. RGB / HM-10 / ESP32 drivers can share this
 * so the physical UART backend swaps independently of framing logic.
 */
#ifndef UART_PORT_H
#define UART_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct uart_api {
    int (*open)(int uart_id, uint32_t baud);
    /** Non-blocking read; returns bytes copied (0 if none). */
    int (*read)(int uart_id, uint8_t *buf, size_t max_len);
    int (*write)(int uart_id, const uint8_t *buf, size_t len);
    void (*close)(int uart_id);
} uart_api_t;

void uart_bind(const uart_api_t *api);
const uart_api_t *uart_api(void);

#endif /* UART_PORT_H */
