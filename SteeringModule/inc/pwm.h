#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include "definitions.h"

typedef enum {
    MOTOR_DIR_IDX = 1,
    MOTOR_BRAKE_IDX,
    MOTOR_SPEED_IDX,
    SERVO_PULSE_WIDTH_HB_IDX,
    SERVO_PULSE_WIDTH_LB_IDX
} MOTOR_SERVO_PAYLOAD_IDX;

typedef struct {
    bool motor_dir;
    bool motor_brake;
    uint8_t motor_speed;
    uint16_t servo_pulse_width;
    bool new_data_is_ready;
} MOTOR_SERVO_DATA;

typedef enum {
    DT_IDX = 1,
    SENSOR_IN_H_IDX,
    SENSOR_IN_L_IDX,
    WANTED_SENSOR_IN_H_IDX,
    WANTED_SENSOR_IN_L_IDX,
    SENSOR_MODE_IDX
} PID_DATA_PAYLOAD_IDX;

typedef enum {
    K_P_IDX = 1,
    K_D_IDX,
} PID_PARAM_PAYLOAD_IDX; 

typedef enum {
    LINE_DISTANCE, 
    JUNCTION_DISTANCE
} SENSOR_MODES;

typedef struct {
    uint8_t dt; // time since last pd-calculation in milliseconds
    uint8_t sensor_mode;
    int16_t sensor_in;
    int16_t wanted_in;
    int16_t prev_error;
    bool new_data_is_ready;
} PID_DATA;

typedef struct {
    uint8_t k_p;
    uint8_t k_d;
} PID_PARAMS;

// LUT for (65536 / dt_ms) stored in Flash memory.
// Array index = dt in milliseconds (bounded from 0 to 100 ms).
extern const uint16_t dt_reciprocal_LUT[101] PROGMEM;

// init PWM-signal generation
void init_motor();

// Init PWM-signal (50Hz)
void init_servo();

// assign the pid output from "run_new_pid" to the correct registers
void assign_pid_output(int32_t pid_output, uint8_t sensor_mode);

// calculate a new pid output from the most recent pid data, and run "assign_pid_output"
void run_new_pid();

// set the registers that controlls the motor and servo, this function is used when manually sending data that will be assigned to the motor and servo.
void set_motor_servo_params();

// assigns variables in some UART_DATA struct depending on which message_id is given.
void dispatch_data(const uint8_t *payload, uint8_t payload_length);