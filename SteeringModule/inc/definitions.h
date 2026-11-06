#pragma once

#include <stdbool.h>

//UART
#define FOSC 16000000UL // Clock Speed
#define MESSAGE_ID_IDX 0

typedef enum {
    MOTOR_SERVO_MSG_ID = 1,
    PID_DATA_MSG_ID,
    PID_PARAM_MSG_ID,
} MESSAGE_IDS;

//PWM
#define SERVO_TOP_50HZ 39999
#define SERVO_START_PULSE_WIDTH 3000

//PID
#define SHIFT_SCALER_Q_PID 21
#define RECIPROCAL_R_PID ((1UL << SHIFT_SCALER_Q_ANGLE) / DT_MAX_VAL)

// Other
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))