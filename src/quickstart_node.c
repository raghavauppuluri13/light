#include "common.h"
#include <immintrin.h> // SSE/AVX intrinsics
#include <math.h>

#define MAX_ITER 10
#define BAILOUT 2.0f
#define ZOOM 2.0f

void *dora_context;
void cleanup() { LOG("[c node] CLEANED UP!\n"); }

// Quaternion multiplication
void quat_mult(const float *q1, const float *q2, float *result) {
    result[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
    result[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
    result[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
    result[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
}

// Quaternion to matrix
void quat_to_matrix(const float *quat, float matrix[3][3]) {
    float qx = quat[0], qy = quat[1], qz = quat[2], qw = quat[3];
    matrix[0][0] = 1.0f - 2.0f * qy * qy - 2.0f * qz * qz;
    matrix[0][1] = 2.0f * qx * qy - 2.0f * qz * qw;
    matrix[0][2] = 2.0f * qx * qz + 2.0f * qy * qw;

    matrix[1][0] = 2.0f * qx * qy + 2.0f * qz * qw;
    matrix[1][1] = 1.0f - 2.0f * qx * qx - 2.0f * qz * qz;
    matrix[1][2] = 2.0f * qy * qz - 2.0f * qx * qw;

    matrix[2][0] = 2.0f * qx * qz - 2.0f * qy * qw;
    matrix[2][1] = 2.0f * qy * qz + 2.0f * qx * qw;
    matrix[2][2] = 1.0f - 2.0f * qx * qx - 2.0f * qy * qy;
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

// Render fractal with SIMD optimizations
void render_fractal(const Pose *pose, Image *image) {
    float matrix[3][3];
    quat_to_matrix(pose->quat, matrix);

    // Precompute rotation matrix in AVX registers
    __m256 m00 = _mm256_set1_ps(matrix[0][0]);
    __m256 m01 = _mm256_set1_ps(matrix[0][1]);
    __m256 m02 = _mm256_set1_ps(matrix[0][2]);
    __m256 m10 = _mm256_set1_ps(matrix[1][0]);
    __m256 m11 = _mm256_set1_ps(matrix[1][1]);
    __m256 m12 = _mm256_set1_ps(matrix[1][2]);
    __m256 m20 = _mm256_set1_ps(matrix[2][0]);
    __m256 m21 = _mm256_set1_ps(matrix[2][1]);
    __m256 m22 = _mm256_set1_ps(matrix[2][2]);

    // Allocate memory aligned for AVX
    uint8_t *data = image->data;
    int width = CFG_CAMERA_WIDTH;
    int height = CFG_CAMERA_HEIGHT;

    // Process 8 pixels per iteration using AVX
    for (int y = 0; y < height; y++) {
        int x = 0;
        for (; x <= width - 8; x += 8) {
            // Compute fx and fy for 8 pixels
            __m256 fx = _mm256_set_ps((x + 7 - width / 2.0f) / width * ZOOM,
                                      (x + 6 - width / 2.0f) / width * ZOOM,
                                      (x + 5 - width / 2.0f) / width * ZOOM,
                                      (x + 4 - width / 2.0f) / width * ZOOM,
                                      (x + 3 - width / 2.0f) / width * ZOOM,
                                      (x + 2 - width / 2.0f) / width * ZOOM,
                                      (x + 1 - width / 2.0f) / width * ZOOM,
                                      (x + 0 - width / 2.0f) / width * ZOOM);

            __m256 fy_val = _mm256_set1_ps((y - height / 2.0f) / height * ZOOM);
            __m256 fy =
                _mm256_set_ps(fy_val[7], fy_val[6], fy_val[5], fy_val[4],
                              fy_val[3], fy_val[2], fy_val[1], fy_val[0]);

            // Ray direction before rotation: (fx, fy, -1.0)
            __m256 ray_z = _mm256_set1_ps(-1.0f);

            // Apply rotation matrix
            __m256 dir_x = _mm256_add_ps(
                _mm256_add_ps(_mm256_mul_ps(m00, fx), _mm256_mul_ps(m01, fy)),
                _mm256_mul_ps(m02, ray_z));

            __m256 dir_y = _mm256_add_ps(
                _mm256_add_ps(_mm256_mul_ps(m10, fx), _mm256_mul_ps(m11, fy)),
                _mm256_mul_ps(m12, ray_z));

            __m256 dir_z = _mm256_add_ps(
                _mm256_add_ps(_mm256_mul_ps(m20, fx), _mm256_mul_ps(m21, fy)),
                _mm256_mul_ps(m22, ray_z));

            // Normalize directions
            __m256 len_sq =
                _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(dir_x, dir_x),
                                            _mm256_mul_ps(dir_y, dir_y)),
                              _mm256_mul_ps(dir_z, dir_z));
            __m256 inv_len =
                _mm256_rsqrt_ps(len_sq); // Approximate reciprocal sqrt

            // Refine reciprocal sqrt
            inv_len =
                _mm256_mul_ps(_mm256_set1_ps(1.0f),
                              _mm256_fnmadd_ps(_mm256_mul_ps(len_sq, inv_len),
                                               _mm256_set1_ps(0.5f), inv_len));

            dir_x = _mm256_mul_ps(dir_x, inv_len);
            dir_y = _mm256_mul_ps(dir_y, inv_len);
            dir_z = _mm256_mul_ps(dir_z, inv_len);

            // Store normalized directions
            float dirs[8][3];
            _mm256_storeu_ps(&dirs[0][0], dir_x);
            _mm256_storeu_ps(&dirs[0][1], dir_y);
            _mm256_storeu_ps(&dirs[0][2], dir_z);

            // Perform ray marching for each of the 8 pixels
            uint8_t colors[8];
            for (int i = 0; i < 8; i++) {
                float distance =
                    ray_march(pose->pos[0], pose->pos[1], pose->pos[2],
                              dirs[i][0], dirs[i][1], dirs[i][2]);
                colors[i] =
                    (distance < MAX_ITER)
                        ? (255 - (uint8_t)(distance * 255.0f / MAX_ITER))
                        : 0;
            }

            // Store the results
            for (int i = 0; i < 8; i++) {
                data[y * width + x + i] = colors[i];
            }
        }

        // Handle remaining pixels
        for (; x < width; x++) {
            float fx = (x - width / 2.0f) / width * ZOOM;
            float fy = (y - height / 2.0f) / height * ZOOM;
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
            data[y * width + x] = color;
        }
    }
}

int main() {
    LOG("[c node] Hello World\n");
    set_realtime_priority(10);
    atexit(cleanup);

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
        // render_fractal(&view_pose, &fov);

        // send outputs
        DORA_SEND_Image(dora_context, "fov", &fov);
    }
    free_dora_context(dora_context);
    LOG("[c node] finished successfully\n");

    return 0;
}
