#pragma once

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "definitions.h"

typedef void (*function_callback_t)(const uint8_t *data, uint8_t length);

typedef enum {
    PENDING_HEADER, // HEADER IDENTIFIER
    READ_LENGTH,    // LENGTH OF DATA PAYLOAD
    READ_PAYLOAD,   // DATA PAYLOAD (ORDER MUST MATCH THE ORDER IN THE STRUCT WHERE THE DATA WILL FINALLY ARRIVE AT)
    PENDING_FOOTER  // RESULTING XOR-CHECKSUM OF ALL PREVIOUS BYTES (MUST MATCH CALCULATED CHECKSUM AS TO MAKE SURE SENT DATA = RECIEVED DATA)
} UART_STATES;

typedef struct {
    uint8_t temp_rx_buffer[UART_BUFFER_SIZE];
    uint8_t expected_length;
    uint8_t checksum;
    uint8_t message_id;
    uint8_t payload_index;
    UART_STATES state;
} UART_PARSER;


typedef struct {
    bool motor_dir;
    bool motor_brake;
    uint8_t motor_speed;
    uint16_t servo_pulse_width;
} UART_DATA;

// Init UART with given baud rate
void uart_init(uint16_t baud_rate);

// check if there is new data in the uart buffer
uint8_t uart_read_available(uint8_t rx_head, uint8_t rx_tail);

// returns the first unread byte in the recieve buffer
uint8_t read_uart_byte(volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_BUFFER_SIZE]);

// assigns variables in some UART_DATA struct depending on which message_id is given.
void dispatch_data(const uint8_t *payload, uint8_t payload_length);

// process and parse data recieved from uart
void process_uart_data(void *context, uint8_t byte);