#include "board_bind.h"

#include "config/drivers_select.h"
#include "ports/uart_port.h"
#include "ports/rgb_sensor_port.h"
#include "ports/sensor_x_port.h"
#include "ports/status_transport_port.h"

int board_bind_and_init(void)
{
    uart_bind(uart_stub_api());
    rgb_sensor_bind(rgb_sensor_stub_api());
    sensor_x_bind(sensor_x_stub_api());
    status_transport_bind(status_transport_stub_api());

    if (rgb_sensor_init() != 0) {
        return -1;
    }
    if (sensor_x_init() != 0) {
        return -1;
    }
    if (status_transport_init() != 0) {
        return -1;
    }
    return 0;
}
