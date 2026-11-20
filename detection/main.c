
#include <stdint.h>
#include <unistd.h>
#include "drivers/rgb_sensor.h"
#include "drivers/ble_link.h"
#include "app/sensor_data.h"
#include "app/state_machine.h"

#define LOOP_PERIOD_MS 50U     // TEMPORARY VALUE
#define BLE_HEARTBEAT_MS 1000U   // 1 Hz heartbeat to XIAO ESP32

void *mainThread(void *arg0) {
    uint32_t now_ms = 0;
    uint32_t last_hb_ms = 0;
    System_state current_state = STATE_NORMAL;
    System_state prev_state = STATE_NORMAL;

    rgb_sensor_init();
    ble_link_init();
    state_machine_reset();

    while (1) {
        // 1. Poll hardware drivers
        rgb_sensor_poll();
        ble_link_poll(now_ms); // Saves sensor_x.last_update_ms

        // 2. Process raw data into disturbance_count & health
        RgbReading rgb = rgb_sensor_get_latest();
        SensorXReading sx = ble_link_get_sensor_x();
        SensorData sensors = process_sensor_data(rgb, sx, now_ms);

        // 3. Run state machine
        current_state = update_state_machine(current_state, sensors, now_ms);

        // 4. Act on immediate state transitions
        if (current_state != prev_state) {
            if (current_state == STATE_ALARM) {
                ble_link_send_alarm();
            } else if (current_state == STATE_FAULT) {
                ble_link_send_fault();
            }
            prev_state = current_state;
        }

        // 5. Periodic heartbeat
        if ((now_ms - last_hb_ms) >= BLE_HEARTBEAT_MS) {
            if (current_state == STATE_NORMAL || current_state == STATE_SUSPECT) {
                ble_link_send_heartbeat();
            }
            last_hb_ms = now_ms;
        }

        usleep(LOOP_PERIOD_MS * 1000U);
        now_ms += LOOP_PERIOD_MS;
    }
}