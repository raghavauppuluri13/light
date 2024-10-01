#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#ifdef DEBUG
#define LOG(format, ...)                                                       \
    do {                                                                       \
        fprintf(stderr, "%s, %d: ", __FILE__, __LINE__);                       \
        fprintf(stderr, format, ##__VA_ARGS__);                                \
        fprintf(stderr, "\n");                                                 \
    } while (0)
#else
#define LOG(format, ...)                                                       \
    do {                                                                       \
    } while (0)
#endif

#ifdef DEBUG
#define PRINT_BYTES_IN_HEX(data, len)                                          \
    do {                                                                       \
        fprintf(stderr, "%s, %d: \n", __FILE__, __LINE__);                     \
        for (uint8_t i = 0; i < (len); i++) {                                  \
            fprintf(stderr, "%02X ", (data)[i]);                               \
        }                                                                      \
        fprintf(stderr, "\n");                                                 \
    } while (0)
#else
#define PRINT_BYTES_IN_HEX(format, ...)                                        \
    do {                                                                       \
    } while (0)
#endif

#define ASSERT_V(cond, fmt, ...)                                               \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "Assertion failed: ");                             \
            fprintf(stderr, (fmt), __VA_ARGS__);                               \
            fprintf(stderr, "\nIn file %s, line %d\n", __FILE__, __LINE__);    \
            abort();                                                           \
        }                                                                      \
    } while (0)

#endif
