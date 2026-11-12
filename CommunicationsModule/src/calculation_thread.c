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
#include "sensor_data.h"

static sensor_data_t sd;

typedef struct {
    int from_node;        // last node passed (confirmed by the camera)
    int to_node;          // next node on the route
    float dist_on_edge;   // distance driven since from_node (odometry), in cm
    int route_index;      // where in the planned route list we are
} car_state_t;

void *calculation_thread(void *arg)
{
    (void)arg;
    return NULL;
}


void read_data(void *arg)
{
    get_sensor_data(&sd);
    // TODO; Add data from external pc (route)
    (void)arg;
}

void shortest_route(void *arg)
{
    //Calculate shortest route to 
    (void)arg;
}