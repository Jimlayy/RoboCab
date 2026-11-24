/*
 * calculation_thread.h - The thread that decides how the car should drive.
 *
 * Each period it reads the latest command (command.h) and sensor data,
 * decides speed and steering angle, and sends them to the steering module.
 */
#ifndef CALCULATION_THREAD_H
#define CALCULATION_THREAD_H

#include <signal.h>

/* Where the car is on the map */
typedef struct {
    int from_node;        // last node passed (confirmed by the camera)
    int to_node;          // next node on the route
    float dist_on_edge;   // distance driven since from_node (odometry), in cm
    int route_index;      // where in the planned route list we are
} car_state_t;

/* Arguments from main to the calculation thread */
typedef struct {
    int steering_fd;                  // open port to the steering module
    volatile sig_atomic_t *running;   // set to 0 to stop the thread
} calc_args_t;

/* Thread function. Pass a pointer to a calc_args_t that lives as long as the thread. */
void *calculation_thread(void *ctx);

#endif
