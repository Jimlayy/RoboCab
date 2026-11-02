#ifndef ODOMETRY_H
#define ODOMETRY_H

#include <stdbool.h>
#include <stdint.h>

/* Hjulodometri (designspec 5.6.2). Tio magneter per hjul ger tio pulser per varv. */
#define ODOMETRY_PULSES_PER_REV 10

/* s = N / 10 * C. Omkretsen C är ett argument eftersom den ska kalibreras. */
uint32_t odometry_wheel_distance_mm(uint32_t pulses, uint16_t circumference_mm);

/* s_tot = (s_left + s_right) / 2. */
uint32_t odometry_total_distance_mm(uint32_t left_mm, uint32_t right_mm);

/* v = delta_s / delta_t. Ger 0 om delta_ms är 0. */
uint32_t odometry_speed_mm_s(uint32_t delta_mm, uint32_t delta_ms);

/* Sant om senaste pulsen är äldre än timeout_ms. Klarar att ms-räknaren slår runt. */
bool odometry_wheel_stopped(uint32_t now_ms, uint32_t last_pulse_ms,
                            uint32_t timeout_ms);

#endif
