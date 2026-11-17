#pragma once

#include "uart_definitions.h"
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>

//UART
#define MYUBBR ((F_CPU / 8UL + BAUD_RATE / 2) / BAUD_RATE - 1)
#define MESSAGE_ID_IDX 0

void init_custom_stdout();
int uart_putchar(char c, FILE *stream);

typedef enum {
    MOTOR_SERVO_MSG_ID = 1,
    PID_DATA_MSG_ID,
    PID_PARAM_MSG_ID,
} MESSAGE_IDS;

//PWM
#define SERVO_PW_TIME_MS_MIN 1
#define SERVO_PW_TIME_MS_MAX 2
#define SERVO_FREQ 50
#define SERVO_PRESCALER 8
#define SERVO_TOP_50HZ (F_CPU / (SERVO_FREQ * SERVO_PRESCALER) - 1)    
#define SERVO_PULSE_WIDTH_MIN (SERVO_PW_TIME_MS_MIN * F_CPU / (1000UL * SERVO_PRESCALER))
#define SERVO_PULSE_WIDTH_MAX (SERVO_PW_TIME_MS_MAX * F_CPU / (1000UL * SERVO_PRESCALER))
#define SERVO_START_PULSE_WIDTH (SERVO_PULSE_WIDTH_MIN + (SERVO_PULSE_WIDTH_MAX - SERVO_PULSE_WIDTH_MIN) / 2 + 20) 

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