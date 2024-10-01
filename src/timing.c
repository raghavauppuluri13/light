#include "timing.h"

void rk_init(RateState *state) {
    state->interval = 1. / state->rate;
    state->last_monitor_time = seconds_since_boot();
    state->next_frame_time = state->last_monitor_time + state->interval;
}

bool rk_keep_time(RateState *state) {
    int lagged = rk_monitor_time(state);
    if (state->remaining > 0) {
        double tick;
        do {
            tick = seconds_since_boot();
        } while (tick < state->next_frame_time);
    }
    return lagged;
}

bool rk_monitor_time(RateState *state) {
    ++state->frame;
    state->last_monitor_time = seconds_since_boot();
    state->remaining = state->next_frame_time - state->last_monitor_time;

    bool lagged = state->remaining < 0;
    if (lagged) {
        if (state->print_delay_threshold > 0 &&
            state->remaining < -state->print_delay_threshold) {
            LOG("lagging by %.2f ms", -state->remaining * 1000);
        }
        state->next_frame_time = state->last_monitor_time + state->interval;
    } else {
        state->next_frame_time += state->interval;
    }
    return lagged;
}
