#pragma once

#include "uart_definitions.h"
#include <stdbool.h>

//UART
#define MYUBBR ((F_CPU / 16 + BAUD_RATE / 2) / BAUD_RATE - 1)
#define MESSAGE_ID_IDX 0

typedef enum {
    MOTOR_SERVO_MSG_ID = 1,
    PID_DATA_MSG_ID,
    PID_PARAM_MSG_ID,
} MESSAGE_IDS;

//PWM
#define SERVO_FREQ 50
#define SERVO_PRESCALER 8
#define SERVO_TOP_50HZ (F_CPU / (SERVO_FREQ * SERVO_PRESCALER) - 1)    
#define SERVO_START_PULSE_WIDTH 1500    // 1500 ticks (1 tick per microsecond => 1.5 ms)
#define SERVO_PULSE_WIDTH_MIN 1000
#define SERVO_PULSE_WIDTH_MAX 2000

//PID
#define SHIFT_SCALER_Q_PID 21
#define RECIPROCAL_R_PID ((1UL << SHIFT_SCALER_Q_ANGLE) / DT_MAX_VAL)

//LCD
// ---------- THESE ARE FOR SIMULATION ON ATMEGA328 ---------- //
#define LCD_PORTC_OUT 0x78
#define LCD_PORTB_OUT 0x3C
#define ENABLE_HIGH_328     PORTC |= (1 << PC2); _delay_us(1)
#define ENABLE_LOW_328     PORTC &= ~(1 << PC2); _delay_us(1)
// ----------------------------------------------------------- //

#define ENABLE_HIGH     PORTD |= (1 << PD3); _delay_us(1)
#define ENABLE_LOW      PORTD &= ~(1 << PD3); _delay_us(1)

#define INSTRUCTION 0
#define DATA 1

// Other
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))