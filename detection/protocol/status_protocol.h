/**
 * 2-bit outbound status to XIAO ESP32 via Bluetooth/UART bridge.
 *
 *   00 Heartbeat
 *   01 ALARM
 *   10 FAULT
 *   11 reserved
 */
#ifndef STATUS_PROTOCOL_H
#define STATUS_PROTOCOL_H

#include <stdint.h>

typedef enum {
    STATUS_HEARTBEAT = 0x0, /* 00 */
    STATUS_ALARM     = 0x1, /* 01 */
    STATUS_FAULT     = 0x2, /* 10 */
    STATUS_RESERVED  = 0x3  /* 11 — do not send */
} StatusCode;

/** Pack status into a wire byte (low 2 bits; upper bits zero). */
uint8_t status_to_wire(StatusCode code);

/** Decode low 2 bits; unknown values map to STATUS_RESERVED. */
StatusCode status_from_wire(uint8_t byte);

const char *status_name(StatusCode code);

#endif /* STATUS_PROTOCOL_H */
