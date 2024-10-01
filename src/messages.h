#ifndef MESSAGES_H

#include "constants.h"

typedef struct {
    float sin[CFG_CAN_NUM_DEVICES];
    float cos[CFG_CAN_NUM_DEVICES];
    float tan[CFG_CAN_NUM_DEVICES];
} FunctionData;
#endif
