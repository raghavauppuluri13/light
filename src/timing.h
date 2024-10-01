#ifndef TIMING_H
#define TIMING_H

#include "log.h"
#include <sched.h> // For sched_param, sched_setscheduler, SCHED_FIFO
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h> // For syscall, SYS_gettid
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// timespec utility
static inline double diff_timespec(const struct timespec *time1,
                                   const struct timespec *time0) {
    return (time1->tv_sec - time0->tv_sec) +
           (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}

// time keeping
typedef struct {
    double interval;
    double next_frame_time;
    double last_monitor_time;
    double remaining;
    double rate;
    float print_delay_threshold;
    uint64_t frame;
} RateState;

void rk_init(RateState *state);
bool rk_keep_time(RateState *state);
bool rk_monitor_time(RateState *state);

// time helpers
static inline uint64_t nanos_since_boot() {
    struct timespec t;
    clock_gettime(CLOCK_BOOTTIME, &t);
    return t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static inline double millis_since_boot() {
    struct timespec t;
    clock_gettime(CLOCK_BOOTTIME, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec * 1e-6;
}

static inline double seconds_since_boot() {
    struct timespec t;
    clock_gettime(CLOCK_BOOTTIME, &t);
    return (double)t.tv_sec + t.tv_nsec * 1e-9;
}

static inline uint64_t nanos_since_epoch() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static inline double seconds_since_epoch() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (double)t.tv_sec + t.tv_nsec * 1e-9;
}

static inline uint64_t nanos_monotonic() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static inline uint64_t nanos_monotonic_raw() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static inline int set_realtime_priority(int level) {
#ifdef __linux__
    long tid = syscall(SYS_gettid);

    // should match python using chrt
    struct sched_param sa;
    memset(&sa, 0, sizeof(sa));
    sa.sched_priority = level;
    return sched_setscheduler(tid, SCHED_FIFO, &sa);
#else
    return -1;
#endif
}

#endif
