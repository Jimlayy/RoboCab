#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>
#include <time.h>

/* Latest values received from the sensor module.
 * Field layout follows PKT_SENSOR in protocol.h (format not yet decided). */
typedef struct {
    uint16_t distance;
    uint16_t speed_left;
    uint16_t speed_right;
    int16_t  gyro_z;
    int16_t  accel_x;
    struct timespec timestamp;   // CLOCK_MONOTONIC time the packet arrived
} sensor_data_t;

/* Store a new sensor reading. Thread-safe. */
void sensor_data_update(const sensor_data_t *s);

/* Copy the latest sensor reading into *out. Thread-safe.
 * Returns 0 on success, -1 if no reading has been received yet. */
int sensor_data_get(sensor_data_t *out);

#endif
