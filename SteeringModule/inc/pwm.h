#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

// init PWM-signal generation
void init_motor();

void set_motor_params(bool direction, uint8_t pulse_width);

// Init PWM-signal (50Hz)
void init_servo(volatile UART_DATA *uart_data);

// uint16_t calculate_servo_pulse_width(uint8_t angle);

void set_servo_params(uint16_t pulse_width);