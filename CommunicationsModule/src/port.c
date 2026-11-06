#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include "../include/port.h"
#include "../../Common/parser.h"
#include "../include/packet_handler.h"

static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

static speed_t to_speed(int baud)
{
    switch (baud) {
    case 9600:   return B9600;
    case 19200:  return B19200;
    case 38400:  return B38400;
    case 57600:  return B57600;
    case 115200: return B115200;
    default:     return 0;
    }
}

int open_port(const char *dev, int baud)
{
    speed_t speed = to_speed(baud);
    if (!speed) {
        fprintf(stderr, "Unsupported baud rate %d\n", baud);
        return -1;
    }

    // Read AND write on the same descriptor
    int fd = open(dev, O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", dev, strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) { perror("tcgetattr"); close(fd); return -1; }
    cfmakeraw(&tty);               // raw bytes, 8N1
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cc[VMIN]  = 0;           // poll() does the waiting
    tty.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) { perror("tcsetattr"); close(fd); return -1; }
    tcflush(fd, TCIOFLUSH);

    return fd;
}

int poll_port(int fd, volatile sig_atomic_t *running)
{
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    parser_t package_context;
    parser_init(&package_context, save_packet);

    while (*running) {
        int r = poll(&pfd, 1, 500);

        if (r < 0) {
            if (errno == EINTR) continue;      // interrupted by a signal, try again
            perror("poll");
            return -1;
        }
        if (r == 0) {
            fprintf(stderr, "No data on port for 500 ms\n");
            continue;                          // timeout: just wait again
        }
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "Port lost (cable unplugged?)\n");
            return -1;
        }
        if (pfd.revents & POLLIN) {
            uint8_t buf[64];
            ssize_t n = read(fd, buf, sizeof buf);
            if (n < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;
                perror("read");
                return -1;
            }
            for (ssize_t i = 0; i < n; i++)
                parser_feed(&package_context ,buf[i]);
        }
    }
    return 0;
}

// write() may send fewer bytes than asked for, so keep going until all are sent
int write_all(int fd, const uint8_t *buf, size_t n)
{
    int result = 0;
    size_t sent = 0;

    pthread_mutex_lock(&write_lock);         // one whole frame at a time
    while (sent < n) {
        ssize_t w = write(fd, buf + sent, n - sent);
        if (w < 0) {
            if (errno == EINTR) continue;    // interrupted by a signal, try again
            perror("write");
            result = -1;
            break;                           // break, so the mutex still gets unlocked
        }
        sent += (size_t)w;
    }
    pthread_mutex_unlock(&write_lock);
    return result;
}