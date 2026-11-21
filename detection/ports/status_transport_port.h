/**
 * Outbound 2-bit status transport to XIAO ESP32 (UART / BT bridge).
 */
#ifndef STATUS_TRANSPORT_PORT_H
#define STATUS_TRANSPORT_PORT_H

#include "protocol/status_protocol.h"

typedef struct status_transport_api {
    int (*init)(void);
    int (*send)(StatusCode code);
} status_transport_api_t;

void status_transport_bind(const status_transport_api_t *api);
const status_transport_api_t *status_transport_api(void);

int status_transport_init(void);
int status_transport_send(StatusCode code);

#endif /* STATUS_TRANSPORT_PORT_H */
