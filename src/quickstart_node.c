#include "common.h"
#include <math.h>

#define FunctionData_LEN 10
#define FunctionData_FIELDS 3
typedef struct {
    float sin[FunctionData_LEN];
    float cos[FunctionData_LEN];
    float tan[FunctionData_LEN];
} FunctionData;

void *dora_context;
void cleanup() { LOG("[c node] CLEANED UP!\n"); }

int main() {
    set_realtime_priority(10);
    atexit(cleanup);

    LOG("[c node] Hello World\n");
    dora_context = init_dora_context_from_env();
    if (dora_context == NULL) {
        LOG("failed to init dora context\n");
        return -1;
    }
    LOG("[c node] dora context initialized\n");

    FunctionData fdata = {0};

    struct timespec now, start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = diff_timespec(&now, &start);

    while (1) {
        void *event = dora_next_event(dora_context); // blocking
        if (event == NULL) {
            LOG("[c node] ERROR: unexpected end of event\n");
            return -1;
        }

        enum DoraEventType ty = read_dora_event_type(event);

        // process inputs
        if (ty == DoraEventType_Input) {
            char *input_id;
            size_t input_id_len;
            read_dora_input_id(event, &input_id, &input_id_len);
            if (strcmp(input_id, "tick") == 0) {
                clock_gettime(CLOCK_MONOTONIC, &now);
                elapsed = diff_timespec(&now, &start);
            } else {
                LOG("[c node] received unexpected input: %s\n", input_id);
            }
        } else if (ty == DoraEventType_Stop) {
            break;
            LOG("[c node] received stop event\n");
        } else {
            break;
            LOG("[c node] received unexpected event: %d\n", ty);
        }
        free_dora_event(event);

        // main
        for (int i = 0; i < FunctionData_LEN; ++i) {
            fdata.sin[i] = sin(elapsed);
            fdata.cos[i] = cos(elapsed);
            fdata.tan[i] = tan(elapsed);
        }
        dora_send_array_struct(dora_context, "function_data", &fdata,
                               FunctionData_LEN, FunctionData_FIELDS);
    }
    free_dora_context(dora_context);
    LOG("[c node] finished successfully\n");

    return 0;
}
