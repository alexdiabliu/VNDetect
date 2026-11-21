/**
 * Placeholder Capstone rules (features only — no drivers):
 *  - FAULT if any sensor unhealthy
 *  - ALARM if disturbance_count >= 2, or SUSPECT longer than CFG_SUSPECT_TIME_MS
 *  - HEARTBEAT while NORMAL/SUSPECT (caller gates period)
 *  - ALARM latches once entered
 */
#include "think.h"
#include "config/board_config.h"

static uint32_t s_suspect_start_ms;

void think_reset(void)
{
    s_suspect_start_ms = 0;
}

static ThinkResult make_result(ThinkState prev, ThinkState next, StatusCode status, bool publish)
{
    ThinkResult r;
    r.state = next;
    r.status = status;
    r.publish = publish;
    r.state_changed = (prev != next);
    return r;
}

ThinkResult think_step(ThinkState current, const ThinkInputs *in)
{
    const SensorFeatures *f;
    uint32_t now;

    if (!in) {
        return make_result(current, THINK_STATE_FAULT, STATUS_FAULT, true);
    }

    f = &in->features;
    now = in->now_ms;

    /* Latch ALARM */
    if (current == THINK_STATE_ALARM) {
        return make_result(current, THINK_STATE_ALARM, STATUS_ALARM, false);
    }

    if (!f->all_sensors_operational) {
        return make_result(current, THINK_STATE_FAULT, STATUS_FAULT, true);
    }

    switch (current) {
    case THINK_STATE_NORMAL:
        if (f->disturbance_count >= 2) {
            return make_result(current, THINK_STATE_ALARM, STATUS_ALARM, true);
        }
        if (f->disturbance_count == 1) {
            s_suspect_start_ms = now;
            return make_result(current, THINK_STATE_SUSPECT, STATUS_HEARTBEAT, false);
        }
        return make_result(current, THINK_STATE_NORMAL, STATUS_HEARTBEAT, false);

    case THINK_STATE_SUSPECT:
        if (f->disturbance_count >= 2 ||
            (now - s_suspect_start_ms) >= CFG_SUSPECT_TIME_MS) {
            return make_result(current, THINK_STATE_ALARM, STATUS_ALARM, true);
        }
        if (f->disturbance_count == 0) {
            return make_result(current, THINK_STATE_NORMAL, STATUS_HEARTBEAT, false);
        }
        return make_result(current, THINK_STATE_SUSPECT, STATUS_HEARTBEAT, false);

    case THINK_STATE_FAULT:
    default:
        return make_result(current, THINK_STATE_FAULT, STATUS_FAULT, false);
    }
}
