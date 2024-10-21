#include "usbcan.h"
#ifdef CFG_USBCAN_ENABLE

/*
 * A Multithreaded SocketCan Driver for supports read/write from multiple can
 * interfaces
 */

SocketCanIface can_ifaces[CFG_CAN2USB_NUM_IFACES];
SocketCanReadBuffer can_rx_buf = {.st_frames = {0}, .stop = 0};
int canid2iface[256] = {0};

void canusb_init() {
    for (int cface_id = 0; cface_id < CFG_CAN2USB_NUM_IFACES; cface_id++) {
        SocketCanIface *cface = &can_ifaces[cface_id];

        // populate canid -> iface mapping
        for (int i = 0; i < CFG_CAN2USB_NUM_IFACES; i++) {
            for (int j = CFG_CAN2USB_IFACE_SLICES[i];
                 j < CFG_CAN2USB_IFACE_SLICES[i + 1]; j++) {
                canid2iface[CFG_CAN_IDS[j]] = i;
                LOG("canid2iface[%d] = %d\n", CFG_ROBOT_MOTOR_IDS[j], i);
            }
        }

        LOG("SocketCAN init\r\n");
        if ((cface->sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            perror("Socket");
            exit(EXIT_FAILURE);
        }
        // Set the socket to nonblocking mode
        int flags = fcntl(cface->sock, F_GETFL, 0);
        if (flags < 0) {
            perror("Fcntl get flags");
            exit(EXIT_FAILURE);
        }
        if (fcntl(cface->sock, F_SETFL, flags | O_NONBLOCK) < 0) {
            perror("Fcntl set nonblocking");
            exit(EXIT_FAILURE);
        }

        strcpy(cface->ifr.ifr_name, CFG_CAN2USB_IFACE_NAMES[cface_id]);
        ioctl(cface->sock, SIOCGIFINDEX, &cface->ifr);

        memset(&cface->addr, 0, sizeof(cface->addr));
        cface->addr.can_family = AF_CAN;                  // set protocol family
        cface->addr.can_ifindex = cface->ifr.ifr_ifindex; // set interface index

        pthread_mutex_init(&cface->mtx, NULL);
        LOG("SocketCAN iface #%d init complete!\r\n", cface_id);
    }

    LOG("SocketCanReadBuffer init\r\n");
    __canusb_start_reading();
    LOG("SocketCanReadBuffer init complete!\r\n");
}

int canusb_send_frame(struct can_frame *frame, SocketCanIface *can_iface) {
    pthread_mutex_lock(&can_iface->mtx);
    ssize_t nbytes =
        sendto(can_iface->sock, frame, sizeof(struct can_frame), 0,
               (struct sockaddr *)&can_iface->addr, sizeof(can_iface->addr));
    pthread_mutex_unlock(&can_iface->mtx);
    if (nbytes < 0) {
        LOG("write err: %s\n", strerror(errno));
        return 0;
    }
    LOG("sent %ld bytes\n", nbytes);
    return 1;
}

int __canusb_recv_frame(struct can_frame *frame, SocketCanIface *can_iface) {
    pthread_mutex_lock(&can_iface->mtx);
    ssize_t nbytes = read(can_iface->sock, frame, sizeof(struct can_frame));
    pthread_mutex_unlock(&can_iface->mtx);
    if (nbytes < 0) {
        // LOG("read err: %s\n", strerror(errno));
        return 0;
    }
    if (nbytes < sizeof(struct can_frame)) {
        LOG("read: incomplete CAN frame, err: %s\n", strerror(errno));
        return 0;
    }
    return 1;
}

void __canusb_start_reading() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    for (int i = 0; i < 256; i++) {
        can_rx_buf.st_frames[i].stamp = now;
    }
    pthread_mutex_init(&can_rx_buf.mtx, NULL);
    if (pthread_create(&can_rx_buf.th, NULL, __canusb_read_threadsafe, NULL) !=
        0) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
}

void *__canusb_read_threadsafe(void *arg) {
    struct can_frame frame = {0};

    while (!atomic_load(&can_rx_buf.stop)) {
        for (int i = 0; i < CFG_CAN2USB_NUM_IFACES; i++) {
            int res = __canusb_recv_frame(&frame, &can_ifaces[i]);
            if (!res) {
                continue;
            }
            uint8_t answer_can_id = (frame.can_id & 0xFF00) >> 8;
            pthread_mutex_lock(&can_rx_buf.mtx);
            can_rx_buf.st_frames[answer_can_id].fr = frame;
            clock_gettime(CLOCK_MONOTONIC,
                          &can_rx_buf.st_frames[answer_can_id].stamp);
            pthread_mutex_unlock(&can_rx_buf.mtx);
        }
    }
    return NULL;
}
void __canusb_stop_reading() {
    atomic_store(&can_rx_buf.stop, 1);
    if (pthread_join(can_rx_buf.th, NULL) != 0) {
        perror("Failed to join motor read thread");
        exit(EXIT_FAILURE);
    }
}

void canusb_get_frame(uint8_t can_id, StampedCanFrame *st_frame) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    pthread_mutex_lock(&can_rx_buf.mtx);
    double elapsed = diff_timespec(&now, &can_rx_buf.st_frames[can_id].stamp);

    // LOG("elapsed %.03f", elapsed);
    can_rx_buf.st_frames[can_id].is_stale =
        elapsed > CFG_CAN2USB_READ_STALE_TIME;
    *st_frame = can_rx_buf.st_frames[can_id];
    pthread_mutex_unlock(&can_rx_buf.mtx);
}

void canusb_destroy() {
    __canusb_stop_reading();
    for (int i = 0; i < CFG_CAN2USB_NUM_IFACES; i++) {
        if (close(can_ifaces[i].sock) < 0) {
            perror("Close");
            exit(EXIT_FAILURE);
        }
    }
}

// helper functions

int float_to_uint(float x, float x_min, float x_max, int bits) {
    float span = x_max - x_min;
    float offset = x_min;
    if (x > x_max)
        x = x_max;
    else if (x < x_min)
        x = x_min;
    return (int)((x - offset) * ((float)((1 << bits) - 1)) / span);
}

float uint_to_float(uint16_t x, float x_min, float x_max) {
    uint16_t type_max = 0xFFFF;
    float span = x_max - x_min;
    return (float)x / type_max * span + x_min;
}

void print_can_packet(uint32_t id, uint8_t *data, uint8_t len) {
    // Print the ID in hexadecimal format
    LOG("ID : %08X\n", id);
    LOG("Data : ");
    PRINT_BYTES_IN_HEX(data, len);
}
#endif
