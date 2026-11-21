/**
 * Compile-time backend selection.
 * Include this from platform/board_bind.c only.
 *
 * To swap hardware: change which driver headers/APIs are declared here
 * and which .c files the Makefile links (see root README).
 */
#ifndef DRIVERS_SELECT_H
#define DRIVERS_SELECT_H

/* Active backends for this build (stubs = host / no hardware). */
#define DRV_RGB_BACKEND          "stub"
#define DRV_SENSOR_X_BACKEND     "stub"
#define DRV_STATUS_BACKEND       "stub"
#define DRV_UART_BACKEND         "stub"

/* Extern vtable constructors provided by the selected driver .c files. */
const struct rgb_sensor_api *rgb_sensor_stub_api(void);
const struct sensor_x_api *sensor_x_stub_api(void);
const struct status_transport_api *status_transport_stub_api(void);
const struct uart_api *uart_stub_api(void);

#endif /* DRIVERS_SELECT_H */
