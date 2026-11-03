#include <avr/io.h>
#include <avr/interrupt.h>

#include "definitions.h"

// init PWM-signal generation with given pulse_width
void init_motor(uint8_t pulse_width);

void set_motor_params(bool direction, uint8_t pulse_width);

// Init PWM-signal (50Hz) with given pulse_width
void init_servo(uint16_t pulse_width);

void set_servo_params(uint8_t pulse_width);