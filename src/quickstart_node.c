#include "common.h"
#include <math.h>

#define MAX_ITER 10
#define BAILOUT 2.0f
#define ZOOM 2.0f

void *dora_context;
void cleanup() { LOG("[c node] CLEANED UP!\n"); }

// Quaternion multiplication
void quat_mult(float *q1, float *q2, float *result) {
    result[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
    result[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
    result[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
    result[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
}

// Quaternion to matrix
void quat_to_matrix(float *quat, float matrix[3][3]) {
    float qx = quat[0], qy = quat[1], qz = quat[2], qw = quat[3];
    matrix[0][0] = 1 - 2 * qy * qy - 2 * qz * qz;
    matrix[0][1] = 2 * qx * qy - 2 * qz * qw;
    matrix[0][2] = 2 * qx * qz + 2 * qy * qw;
    matrix[1][0] = 2 * qx * qy + 2 * qz * qw;
    matrix[1][1] = 1 - 2 * qx * qx - 2 * qz * qz;
    matrix[1][2] = 2 * qy * qz - 2 * qx * qw;
    matrix[2][0] = 2 * qx * qz - 2 * qy * qw;
    matrix[2][1] = 2 * qy * qz + 2 * qx * qw;
    matrix[2][2] = 1 - 2 * qx * qx - 2 * qy * qy;
}

// Mandelbulb distance estimator
float mandelbulb_distance(float x, float y, float z) {
    float dr = 1.0f, r = 0.0f;
    float theta, phi, zr;
    float zx = x, zy = y, zz = z;
    for (int i = 0; i < MAX_ITER; i++) {
        r = sqrtf(zx * zx + zy * zy + zz * zz);
        if (r > BAILOUT)
            break;
        theta = acosf(zz / r);
        phi = atan2f(zy, zx);
        zr = powf(r, 8.0f);
        zx = zr * sinf(theta * 8.0f) * cosf(phi * 8.0f) + x;
        zy = zr * sinf(theta * 8.0f) * sinf(phi * 8.0f) + y;
        zz = zr * cosf(theta * 8.0f) + z;
        dr = powf(r, 7.0f) * dr + 1.0f;
    }
    return 0.5f * logf(r) * r / dr;
}
// Ray marching for fractal rendering
float ray_march(float x, float y, float z, float dx, float dy, float dz) {
    float total_dist = 0.0f;
    for (int i = 0; i < MAX_ITER; i++) {
        float distance = mandelbulb_distance(x, y, z);
        if (distance < 0.001f)
            return total_dist;
        total_dist += distance;
        x += distance * dx;
        y += distance * dy;
        z += distance * dz;
    }
    return total_dist;
}

// Render fractal
void render_fractal(const Pose *pose, Image *image) {
    float matrix[3][3];
    quat_to_matrix((float *)pose->quat, matrix);

    LOG("here");
    for (int y = 0; y < CFG_CAMERA_HEIGHT; y++) {
        for (int x = 0; x < CFG_CAMERA_WIDTH; x++) {
            float fx = (x - CFG_CAMERA_WIDTH / 2.0f) / CFG_CAMERA_WIDTH * ZOOM;
            float fy =
                (y - CFG_CAMERA_HEIGHT / 2.0f) / CFG_CAMERA_HEIGHT * ZOOM;
            float ray_dir[3] = {fx, fy, -1.0f};
            // Apply rotation from the quaternion
            float dir[3];
            dir[0] = matrix[0][0] * ray_dir[0] + matrix[0][1] * ray_dir[1] +
                     matrix[0][2] * ray_dir[2];
            dir[1] = matrix[1][0] * ray_dir[0] + matrix[1][1] * ray_dir[1] +
                     matrix[1][2] * ray_dir[2];
            dir[2] = matrix[2][0] * ray_dir[0] + matrix[2][1] * ray_dir[1] +
                     matrix[2][2] * ray_dir[2];

            // Normalize ray direction
            float len =
                sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
            dir[0] /= len;
            dir[1] /= len;
            dir[2] /= len;

            // Perform ray marching from camera position
            float distance = ray_march(pose->pos[0], pose->pos[1], pose->pos[2],
                                       dir[0], dir[1], dir[2]);

            // Convert distance to grayscale
            uint8_t color =
                (distance < MAX_ITER)
                    ? (255 - (uint8_t)(distance * 255.0f / MAX_ITER))
                    : 0;
            image->data[y * CFG_CAMERA_WIDTH + x] = color;
        }
    }

    LOG("here3");
}

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

    struct timespec now, start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = diff_timespec(&now, &start);

    Image fov = {0};
    Pose view_pose = {0};

    while (1) {
        void *event = dora_next_event(dora_context); // blocking
        if (event == NULL) {
            LOG("[c node] ERROR: unexpected end of event\n");
            return -1;
        }

        enum DoraEventType ty = read_dora_event_type(event);

        // process inputs (should be very compact and fast)
        if (ty == DoraEventType_Input) {
            char *input_id;
            size_t input_id_len;
            read_dora_input_id(event, &input_id, &input_id_len);
            if (strcmp(input_id, "tick") == 0) {
                clock_gettime(CLOCK_MONOTONIC, &now);
                elapsed = diff_timespec(&now, &start);
            } else if (strcmp(input_id, "pose") == 0) {
                DORA_READ_Pose(event, &view_pose);
            } else {
                LOG("[c node] unexpected input: %s\n", input_id);
            }
        } else if (ty == DoraEventType_Stop) {
            break;
            LOG("[c node] received stop event\n");
        } else {
            break;
            LOG("[c node] received unexpected event: %d\n", ty);
        }
        free_dora_event(event); // do *before* computation

        // computation
        LOG("[c node] tick %f\n", elapsed);
        render_fractal(&view_pose, &fov);
        LOG("[c node] tick %f\n", elapsed);

        // send outputs
        DORA_SEND_Image(dora_context, "fov", &fov);
    }
    free_dora_context(dora_context);
    LOG("[c node] finished successfully\n");

    return 0;
}
