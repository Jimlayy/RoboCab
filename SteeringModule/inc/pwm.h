#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "definitions.h"

typedef struct {
    bool motor_dir;
    bool motor_brake;
    uint8_t motor_speed;
    uint16_t servo_pulse_width;
    bool new_data_is_ready;
} MOTOR_SERVO_DATA;

typedef struct {
    uint8_t k_p;
    uint8_t k_d;
    int16_t dt;
    int16_t sensor_input;
    int16_t wanted_input;
    int16_t error;
    int16_t prev_error;
    int16_t output;
    bool new_data_is_ready;
} PID_DATA;


typedef enum {
    MOTOR_DIR_IDX,
    MOTOR_BRAKE_IDX,
    MOTOR_SPEED_IDX,
    SERVO_PULSE_WIDTH_HB_IDX,
    SERVO_PULSE_WIDTH_LB_IDX
} MOTOR_SERVO_PAYLOAD_IDX;

typedef enum {
    K_P_IDX,
    K_D_IDX,
    DT_IDX,
    SENSOR_INPUT_IDX,
    WANTED_INPUT_IDX,
} PID_PAYLOAD_IDX;

// init PWM-signal generation
void init_motor();

// Init PWM-signal (50Hz)
void init_servo();

void set_motor_servo_params();

// assigns variables in some UART_DATA struct depending on which message_id is given.
void dispatch_data(const uint8_t *payload, uint8_t payload_length);