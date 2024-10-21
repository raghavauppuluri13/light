// Auto-generated messages from TOML configuration messages/quickstart.toml at 2024-10-21 11:07:00.796546

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#include "common.h"

#include "constants.h"

// -------------------------------
// Type definition for Pose
// -------------------------------
typedef struct {
	float pos[3];
	float quat[4];
} Pose;
#define DORA_READ_Pose(event, arr) dora_read_f32_array(event, arr, 7)
#define DORA_SEND_Pose(dora_context, id, arr) dora_send_f32_array(dora_context, id, arr, 7)
// -------------------------------
// Type definition for Image
// -------------------------------
typedef struct {
	uint8_t data[CFG_CAMERA_WIDTH * CFG_CAMERA_HEIGHT * 3];
} Image;
#define DORA_READ_Image(event, arr) dora_read_u8_array(event, arr, 27000000)
#define DORA_SEND_Image(dora_context, id, arr) dora_send_u8_array(dora_context, id, arr, 27000000)
#endif // MESSAGES_H
