// USB CAN interface

#ifndef DRIVER_H
#define DRIVER_H

#include "common.h"
#include "constants.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <net/if.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <linux/can.h>
#include <linux/can/raw.h>

/**
 * @brief USB CAN interface.
 */

typedef struct {
    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    pthread_mutex_t mtx;
} SocketCanIface;

typedef struct {
    struct can_frame fr;
    struct timespec stamp;
    int is_stale;
} StampedCanFrame;

typedef struct {
    StampedCanFrame st_frames[256];
    pthread_mutex_t mtx;
    pthread_t th;
    atomic_int stop;
} SocketCanReadBuffer;

void canusb_init();
int canusb_send_frame(struct can_frame *frame, SocketCanIface *can_iface);
int __canusb_recv_frame(struct can_frame *frame, SocketCanIface *can_iface);
void __canusb_start_reading();
void *__canusb_read_threadsafe(void *);
void __canusb_stop_reading();
void canusb_get_frame(uint8_t can_id, StampedCanFrame *data);
void canusb_destroy();

// Helper functions
int float_to_uint(float x, float x_min, float x_max, int bits);
float uint_to_float(uint16_t x, float x_min, float x_max);
void print_can_packet(uint32_t id, uint8_t *data, uint8_t len);

#endif
