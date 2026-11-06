/*
 * main.c - Entry point for the communications module.
 *
 * Opens the USB ports to the sensor module and the steering module,
 * starts one read thread per port and the calculation thread, then waits
 * until the program is stopped with Ctrl+C or a port fails.
 *
 * Usage: ./communications_module [sensor device] [steering device]
 *        defaults: /dev/ttyUSB0 /dev/ttyUSB1
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include "port.h"
#include "uart_definitions.h"

#define SENSOR_DEVICE   "/dev/ttyUSB0"
#define STEERING_DEVICE "/dev/ttyUSB1"

/* Defined in calculation_thread.c. Move this to a header once one exists. */
void *calculation_thread(void *arg);

/* Set to 0 to make every loop in the program stop. */
static volatile sig_atomic_t running = 1;

static void on_signal(int sig)
{
    (void)sig;
    running = 0;
}

/* Reads from one port until the program stops or the port fails. */
static void *port_thread(void *arg)
{
    int fd = *(int *)arg;

    if (poll_port(fd, &running) != 0)
        fprintf(stderr, "Port thread stopped because of an error\n");

    running = 0;    // without both ports there is nothing to do: stop everything
    return NULL;
}

int main(int argc, char *argv[])
{
    const char *sensor_device   = (argc > 1) ? argv[1] : SENSOR_DEVICE;
    const char *steering_device = (argc > 2) ? argv[2] : STEERING_DEVICE;

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int sensor_fd = open_port(sensor_device, BAUD_RATE);
    if (sensor_fd < 0)
        return 1;

    int steering_fd = open_port(steering_device, BAUD_RATE);
    if (steering_fd < 0) {
        close(sensor_fd);
        return 1;
    }

    printf("Sensor: %s, steering: %s. Press Ctrl+C to stop.\n",
           sensor_device, steering_device);

    pthread_t sensor_tid, steering_tid, calc_tid;
    pthread_create(&sensor_tid, NULL, port_thread, &sensor_fd);
    pthread_create(&steering_tid, NULL, port_thread, &steering_fd);
    pthread_create(&calc_tid, NULL, calculation_thread, (void *)&running);

    pthread_join(sensor_tid, NULL);
    pthread_join(steering_tid, NULL);
    pthread_join(calc_tid, NULL);

    close(sensor_fd);
    close(steering_fd);
    printf("Stopped.\n");
    return 0;
}
