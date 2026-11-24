/*
 * File:   odometry.c
 *
 * Wheel odometry calculations. No hardware access here, so the functions can
 * be tested on a PC. Pulse counting and timing live in ../Sensor/Odometry.h.
 */

#include "odometry.h"

// s = N / 10 * C
uint32_t odometry_wheel_distance_mm(uint32_t pulses, uint16_t circumference_mm) {
    uint64_t s = ((uint64_t)pulses * circumference_mm + ODOMETRY_PULSES_PER_REV / 2)
                 / ODOMETRY_PULSES_PER_REV;
    return (s > UINT32_MAX) ? UINT32_MAX : (uint32_t)s;
}

// s_tot = (s_left + s_right) / 2
uint32_t odometry_total_distance_mm(uint32_t left_mm, uint32_t right_mm) {
    return (uint32_t)(((uint64_t)left_mm + right_mm) / 2);
}

// v = delta_s / delta_t, in mm/s
uint32_t odometry_speed_mm_s(uint32_t delta_mm, uint32_t delta_ms) {
    if (delta_ms == 0) {
        return 0;
    }
    uint64_t v = (uint64_t)delta_mm * 1000 / delta_ms;
    return (v > UINT32_MAX) ? UINT32_MAX : (uint32_t)v;
}

bool odometry_wheel_stopped(uint32_t now_ms, uint32_t last_pulse_ms,
                            uint32_t timeout_ms) {
    return (uint32_t)(now_ms - last_pulse_ms) > timeout_ms;
}
