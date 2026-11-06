/*
 * main.c - Entry point for the communications module.
 *
 * Opens the serial port, starts the port thread (reads and parses
 * incoming packets) and the calculation thread, then waits until the
 * program is stopped with Ctrl+C / SIGTERM or the port fails.
 *
 * Usage: ./communications_module [serial device]   (default /dev/serial0)
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/port.h"
#include "../../Common/uart_definitions.h"

#define DEFAULT_DEVICE "/dev/serial0"

/* Defined in calculation_thread.c. Move this to a header once one exists. */
void *calculation_thread(void *arg);

/* Set to 0 to make every loop in the program stop. */
static volatile sig_atomic_t running = 1;

static void on_signal(int sig)
{
    (void)sig;
    running = 0;
}

/* Reads from the port until the program stops or the port fails. */
static void *port_thread(void *arg)
{
    int fd = *(int *)arg;

    if (poll_port(fd, &running) != 0)
        fprintf(stderr, "Port thread stopped because of an error\n");

    running = 0;    // without the port there is nothing to do: stop everything
    return NULL;
}

int main(int argc, char *argv[])
{
    const char *device = (argc > 1) ? argv[1] : DEFAULT_DEVICE;

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int fd = open_port(device, BAUD_RATE);
    if (fd < 0)
        return 1;
    printf("Opened %s at %d baud. Press Ctrl+C to stop.\n", device, BAUD_RATE);

    pthread_t port_tid, calc_tid;

    if (pthread_create(&port_tid, NULL, port_thread, &fd) != 0) {
        perror("pthread_create (port)");
        close(fd);
        return 1;
    }
    if (pthread_create(&calc_tid, NULL, calculation_thread, (void *)&running) != 0) {
        perror("pthread_create (calculation)");
        running = 0;
        pthread_join(port_tid, NULL);
        close(fd);
        return 1;
    }

    pthread_join(port_tid, NULL);
    pthread_join(calc_tid, NULL);

    close(fd);
    printf("Stopped.\n");
    return 0;
}
