// Auto-generated constants from TOML configuration config/quickstart.toml at 2024-09-30 18:27:32.626347

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#define CFG_TOML_PATH "config/quickstart.toml"
// -------------------------------
// Constants for CAN2USB
// -------------------------------
#define CFG_CAN2USB_NUM_IFACES 2
extern const char* CFG_CAN2USB_IFACE_NAMES[];
extern int CFG_CAN2USB_IFACE_SLICES[];
#define CFG_CAN2USB_READ_STALE_TIME 0.1
// -------------------------------
// Constants for CAMERA
// -------------------------------
#define CFG_CAMERA_WIDTH 640
#define CFG_CAMERA_HEIGHT 480
// -------------------------------
// Constants for CAN
// -------------------------------
extern int CFG_CAN_IDS[];
#endif // CONSTANTS_H
