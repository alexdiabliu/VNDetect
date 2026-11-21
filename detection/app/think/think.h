/**
 * Think / decision API — hardware-agnostic.
 */
#ifndef THINK_H
#define THINK_H

#include <stdint.h>
#include <stdbool.h>

#include "protocol/status_protocol.h"
#include "protocol/types.h"

typedef enum {
    THINK_STATE_NORMAL = 0,
    THINK_STATE_SUSPECT,
    THINK_STATE_ALARM,
    THINK_STATE_FAULT
} ThinkState;

typedef struct {
    SensorFeatures features;
    uint32_t now_ms;
} ThinkInputs;

typedef struct {
    ThinkState state;
    StatusCode status;     /* code to publish this tick (if publish) */
    bool publish;          /* true if status should be sent now */
    bool state_changed;
} ThinkResult;

void think_reset(void);
ThinkResult think_step(ThinkState current, const ThinkInputs *in);

#endif /* THINK_H */
