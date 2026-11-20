/*
 * calculation_thread.c - See threads.h for why this thread exists.
 *
 * TODO, in a loop until shared_is_running() is false:
 *   1. read sensor, camera and laptop data from shared_state;
 *   2. new PID parameters -> steering_send_pid_params;
 *   3. new mission -> route_plan_mission;
 *   4. localisation_update; node reached -> steering_send_next_node and
 *      pc_link_send_position; lost -> pc_link_send_status;
 *   5. delta T = time between the last two camera frames;
 *   6. decide_directive -> steering_send_directive;
 *   7. steering_send_lcd_values;
 *   8. logger_data with the values and the decision;
 *   9. sleep until the next period.
 * On exit, send a stop directive so the car is left standing still.
 */
#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include "command.h"
#include "sensor_data.h"
#include "packet_handler.h"
#include <signal.h>
#include <stdbool.h>
//#include "graph.h"

#define PERIOD_MS     50     // 20 decisions per second
#define MANUAL_SPEED  300    // speed when driving by hand (your units)
#define MANUAL_ANGLE  30     // steering angle for LEFT/RIGHT

/* Translate a manual command into speed and steering angle */
static void manual_directive(drive_cmd_t d, bool* motor_dir, bool* motor_brake, int16_t *speed, int16_t *angle)
{
    switch (d) {
    case DRIVE_FORWARD: *speed =  MANUAL_SPEED; *angle = 1500; *motor_dir = 1; *motor_brake = 0;            break;
    case DRIVE_BACK:    *speed =  MANUAL_SPEED; *angle = 1500; *motor_dir = 0; *motor_brake = 0;             break;
    case DRIVE_LEFT:    *speed =  MANUAL_SPEED; *angle = 1500 - MANUAL_ANGLE; *motor_dir = 1; *motor_brake = 0; break;
    case DRIVE_RIGHT:   *speed =  MANUAL_SPEED; *angle = 1500 + MANUAL_ANGLE; *motor_dir = 1; *motor_brake = 0; break;
    default:            *speed = 0;             *angle = 1500;     *motor_dir = 1;     *motor_brake = 1;     break;  // STOP
    }
}

/* Sleep until the next period starts (fixed rate, no drift) */
static void wait_next_period(struct timespec *next)
{
    next->tv_nsec += PERIOD_MS * 1000000L;
    if (next->tv_nsec >= 1000000000L) {
        next->tv_sec++; next->tv_nsec -= 1000000000L; }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, next, NULL);
}

void *calculation_thread(void *ctx)
{
    //context need from main
    calc_args_t *a = ctx; 
    //data acquired from sensors
    command_data_t cmd;
    sensor_data_t  sd;
    //parameters to send to the steering module. Angle is 1000 to 2000 where 1500 is middle.
    bool direction, brake;
    int16_t speed, angle;

    // get the current time and set the clock to it
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (*a->running) {
        command_data_get(&cmd);
        get_sensor_data(&sd);

        if (!cmd.auto_mode) {
            manual_directive(cmd.drive, &direction, &brake, &speed, &angle);
            send_manual_steering(a->steering_fd, direction, brake, speed, angle);
        } else {
            if (cmd.new_mission) {
                // look up cmd.start / cmd.goal in the map, run shortest_path_no_uturn,
                // store the route and reset the route index
            }
            // localisation: advance along the route when the next node is confirmed
            // decide speed and angle from the route, camera and sensor data
            speed = 0; angle = 1500;           // TODO: the real decision
            if (sd.distance < 200)          // obstacle ahead (requirement 23)
                speed = 0;
        }

        //send_steering(a->steering_fd, speed, angle);
        wait_next_period(&next);
    }

    send_steering(a->steering_fd, 0, 0);    // leave the car standing still
    return NULL;
}