/**
 * MSP detection entry — wires ports → features → think → status transport.
 * Host build: `make -C detection` (uses stub drivers).
 */
#include <stdint.h>
#include <stdio.h>

#include "config/board_config.h"
#include "platform/board_bind.h"
#include "ports/rgb_sensor_port.h"
#include "ports/sensor_x_port.h"
#include "ports/status_transport_port.h"
#include "app/features.h"
#include "app/think/think.h"
#include "protocol/types.h"

#if defined(MSP_HOST_BUILD)
#include <unistd.h>
#endif

#ifndef MSP_HOST_MAX_TICKS
#define MSP_HOST_MAX_TICKS 40U /* ~2s at 50ms — enough for a stub smoke run */
#endif

int main(void)
{
    uint32_t now_ms = 0;
    uint32_t last_hb_ms = 0;
    ThinkState state = THINK_STATE_NORMAL;
    unsigned ticks = 0;

    if (board_bind_and_init() != 0) {
#if defined(MSP_HOST_BUILD)
        fprintf(stderr, "board_bind_and_init failed\n");
#endif
        return 1;
    }

    think_reset();

    for (;;) {
        RgbSample rgb;
        SensorXSample sx;
        SensorFeatures features;
        ThinkInputs inputs;
        ThinkResult result;

        rgb_sensor_poll(now_ms);
        sensor_x_poll(now_ms);

        if (rgb_sensor_read(&rgb) != 0 || sensor_x_read(&sx) != 0) {
            state = THINK_STATE_FAULT;
            (void)status_transport_send(STATUS_FAULT);
        } else {
            features = features_from_samples(&rgb, &sx, now_ms);
            inputs.features = features;
            inputs.now_ms = now_ms;
            result = think_step(state, &inputs);
            state = result.state;

            if (result.publish || result.state_changed) {
                if (result.status == STATUS_ALARM || result.status == STATUS_FAULT) {
                    (void)status_transport_send(result.status);
                }
            }

            if ((now_ms - last_hb_ms) >= CFG_HEARTBEAT_PERIOD_MS) {
                if (state == THINK_STATE_NORMAL || state == THINK_STATE_SUSPECT) {
                    (void)status_transport_send(STATUS_HEARTBEAT);
                }
                last_hb_ms = now_ms;
            }
        }

        now_ms += CFG_LOOP_PERIOD_MS;
        ticks++;

#if defined(MSP_HOST_BUILD)
        if (ticks >= MSP_HOST_MAX_TICKS) {
            printf("host smoke run complete: state=%d ticks=%u\n", (int)state, ticks);
            return 0;
        }
        /* Optional real-time pacing; skip for fast CI. */
#else
        (void)ticks;
        /* On MSP: replace with Timer_A / WDT sleep for CFG_LOOP_PERIOD_MS. */
#endif
    }
}
