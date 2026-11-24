#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>
#include <time.h>

/* Latest values received from the sensor module.
 * Field layout follows PKT_SENSOR in protocol.h (format not yet decided). */
typedef struct {
    uint16_t distance;
    uint32_t odo_left_pulse;
    uint32_t odo_right_pulse;
    uint32_t odo_left_tslp;
    uint32_t odo_right_tslp;
    int16_t  accel_x;
    int16_t  gyro_z;
    uint8_t imu_ok;
    struct timespec timestamp;   // CLOCK_MONOTONIC time the packet arrived
} sensor_data_t;

/* Store a new sensor reading. Thread-safe. */
void sensor_data_update(const sensor_data_t *s);

/* Copy the latest sensor reading into *out. Thread-safe.
 * Returns 0 on success, -1 if no reading has been received yet. */
int get_sensor_data(sensor_data_t *out);

#endif
