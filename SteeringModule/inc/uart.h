#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include "definitions.h"

typedef enum {
    PENDING_HEADER,
    UART_MOTOR_DIR,
    UART_MOTOR_SPEED,
    UART_SERVO_ANGLE,
    PENDING_FOOTER
} UART_STATES;

typedef struct {
    bool motor_dir;
    bool temp_motor_dir;
    uint8_t motor_speed;
    uint8_t temp_motor_speed;
    uint8_t servo_angle;
    uint8_t temp_servo_angle;
    uint8_t checksum;
    UART_STATES state;
} UART_DATA;

// Init UART with given baud rate
void uart_init(uint16_t baud_rate);

// check if there is new data in the uart buffer
uint8_t uart_read_available(uint8_t rx_head, uint8_t rx_tail);

// returns the first unread byte in the recieve buffer
uint8_t read_uart_byte(volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_BUFFER_SIZE]);

// process data recieved from uart
void process_uart_data(uint8_t rx_head, volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_BUFFER_SIZE], volatile UART_DATA *uart_data);