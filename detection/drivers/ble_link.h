
#ifndef BLE_LINK_H
#define BLE_LINK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int value;               // Sensor X data
    uint32_t last_update_ms;
    bool valid;
} SensorXReading;

void ble_link_init(void);
void ble_link_poll(uint32_t now_ms);     // Parses incoming Sensor X packets from HM-10 UART
SensorXReading ble_link_get_sensor_x(void);

// Outbound messages to clamp.ino on XIAO ESP32
void ble_link_send_heartbeat(void);
void ble_link_send_alarm(void);
void ble_link_send_fault(void);

#endif