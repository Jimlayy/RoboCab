#ifndef PD_H
#define PD_H

#include <stdint.h>

/* PD-regulator (designspec 3.6.2 och tabell 3):
   u[n] = K_P * e[n] + K_D / dT * (e[n] - e[n-1]), där e[n] = börvärde - ärvärde.

   Räkna i int32_t: int är 16 bitar på ATmega, så K_P * e slår annars över.
   Multiplicera före divisionen, annars blir K_D / dT ofta 0.
   e[n] och u[n] begränsas till int16_t:s intervall. Är dT <= 0 är D-delen 0. */
typedef struct {
    uint8_t kp;
    uint8_t kd;
    int16_t delta_t;
    int16_t actual;
    int16_t setpoint;
    int16_t error;
    int16_t previous_error;
    int16_t output;
} PID_internal_values;

void pd_init(PID_internal_values *pid, uint8_t kp, uint8_t kd);

/* Räknar ut och returnerar ny styrsignal. */
int16_t pd_update(PID_internal_values *pid, int16_t setpoint, int16_t actual,
                  int16_t delta_t);

#endif
