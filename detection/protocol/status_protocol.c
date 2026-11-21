#include "status_protocol.h"

uint8_t status_to_wire(StatusCode code)
{
    return (uint8_t)((uint8_t)code & 0x3U);
}

StatusCode status_from_wire(uint8_t byte)
{
    switch (byte & 0x3U) {
    case 0x0:
        return STATUS_HEARTBEAT;
    case 0x1:
        return STATUS_ALARM;
    case 0x2:
        return STATUS_FAULT;
    default:
        return STATUS_RESERVED;
    }
}

const char *status_name(StatusCode code)
{
    switch (code) {
    case STATUS_HEARTBEAT:
        return "HEARTBEAT";
    case STATUS_ALARM:
        return "ALARM";
    case STATUS_FAULT:
        return "FAULT";
    default:
        return "RESERVED";
    }
}
