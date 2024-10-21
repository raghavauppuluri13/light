#ifndef COMMON_H
#define COMMON_H
#include "../external/dora/apis/c/node/node_api.h"
#include "constants.h"
#include "log.h"
#include "messages.h"
#include "timing.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// sleep
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// math utils

static inline float abs32(float x) {
    uint32_t bits;
    memcpy(&bits, &x, sizeof(bits)); // Copy bits of x into an unsigned integer
    bits &= 0x7FFFFFFF;              // Clear the sign bit
    memcpy(&x, &bits, sizeof(x));    // Copy back to x
    return x;
}
static inline float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

static inline float leaky_integrator(float old, float new_val, float alpha) {
    return old + (new_val - old) * alpha;
}

// dora utility

// u8
static inline void dora_read_u8_array(void *event, void *arr, int arr_len) {
    size_t output_len = 0;
    uint8_t *read;
    read_dora_input_data_u8(event, &read, &output_len);
    assert(output_len == arr_len);
    memcpy(arr, read, sizeof(uint8_t) * arr_len);
}
static inline void dora_send_u8_array(void *dora_context, char *id, void *arr,
                                      int arr_len) {
    char *write[sizeof(uint8_t) * arr_len];
    memcpy(write, arr, sizeof(uint8_t) * arr_len);
    dora_send_output_u8(dora_context, id, strlen(id), (uint8_t *)&write,
                        arr_len);
}

// f32
static inline void dora_send_f32_array(void *dora_context, char *id, void *arr,
                                       int arr_len) {
    char *write[sizeof(float) * arr_len];
    memcpy(write, arr, sizeof(float) * arr_len);
    dora_send_output_f32(dora_context, id, strlen(id), (float *)&write,
                         arr_len);
}

static inline void dora_read_f32_array(void *event, void *arr, int arr_len) {
    size_t output_len = 0;
    float *read;
    read_dora_input_data_f32(event, &read, &output_len);
    assert(output_len == arr_len);
    memcpy(arr, read, sizeof(float) * arr_len);
}

// i32
static inline void dora_send_i32_array(void *dora_context, char *id, void *arr,
                                       int arr_len) {
    char *write[sizeof(int32_t) * arr_len];
    memcpy(write, arr, sizeof(int32_t) * arr_len);
    dora_send_output_i32(dora_context, id, strlen(id), (int32_t *)&write,
                         arr_len);
}

static inline void dora_read_i32_array(void *event, void *arr, int arr_len) {
    size_t output_len = 0;
    int32_t *read;
    read_dora_input_data_i32(event, &read, &output_len);
    assert(output_len == arr_len);
    memcpy(arr, read, sizeof(int32_t) * arr_len);
}

// u64
static inline void dora_send_u64_array(void *dora_context, char *id, void *arr,
                                       int arr_len) {
    char *write[sizeof(uint64_t) * arr_len];
    memcpy(write, arr, sizeof(uint64_t) * arr_len);
    dora_send_output_u64(dora_context, id, strlen(id), (uint64_t *)&write,
                         arr_len);
}

static inline void dora_read_u64_array(void *event, void *arr, int arr_len) {
    size_t output_len = 0;
    uint64_t *read;
    read_dora_input_data_u64(event, &read, &output_len);
    assert(output_len == arr_len);
    memcpy(arr, read, sizeof(uint64_t) * arr_len);
}

#endif
