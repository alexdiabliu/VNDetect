/**
 * Board / system placeholders — no MSP part numbers or pin maps assumed.
 * Replace values as hardware fidelity increases.
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* Loop and keepalive */
#define CFG_LOOP_PERIOD_MS       50U
#define CFG_HEARTBEAT_PERIOD_MS  1000U

/* Sensor health */
#define CFG_SENSOR_TIMEOUT_MS    2000U

/* Think thresholds (placeholder rules) */
#define CFG_SUSPECT_TIME_MS      3000U
#define CFG_RGB_RED_THRESHOLD    180U
#define CFG_SENSOR_X_THRESHOLD   100

/* Placeholder UART identities (not real pins / USCI modules) */
#define CFG_UART_RGB_ID          0
#define CFG_UART_HM10_ID         1
#define CFG_UART_ESP32_ID        2
#define CFG_UART_BAUD_PLACEHOLDER 9600U

#endif /* BOARD_CONFIG_H */
