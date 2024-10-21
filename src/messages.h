// Auto-generated messages from TOML configuration messages/quickstart.toml at
// 2024-10-20 22:58:05.127348

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#include "common.h"

#include "constants.h"

// -------------------------------
// Type definition for Quaternion
// -------------------------------
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Quaternion;
#define DORA_READ_Quaternion(event, arr) dora_read_f32_array(event, arr, 4)
#define DORA_SEND_Quaternion(dora_context, id, arr)                            \
    dora_send_f32_array(dora_context, id, arr, 4)
// -------------------------------
// Type definition for Vector3
// -------------------------------
typedef struct {
    float x;
    float y;
    float z;
} Vector3;
#define DORA_READ_Vector3(event, arr) dora_read_f32_array(event, arr, 3)
#define DORA_SEND_Vector3(dora_context, id, arr)                               \
    dora_send_f32_array(dora_context, id, arr, 3)
// -------------------------------
// Type definition for Image
// -------------------------------
typedef struct {
    uint8_t pix[CFG_CAMERA_WIDTH * CFG_CAMERA_HEIGHT];
} Image;
#define DORA_READ_Image(event, arr) dora_read_u8_array(event, arr, 307200)
#define DORA_SEND_Image(dora_context, id, arr)                                 \
    dora_send_u8_array(dora_context, id, arr, 307200)
#endif // MESSAGES_H
