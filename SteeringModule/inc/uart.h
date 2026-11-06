#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include "definitions.h"
#include "uart_definitions.h"

// Init UART with given baud rate
void uart_init();

// check if there is new data in the uart buffer
uint8_t uart_read_available(uint8_t rx_head, uint8_t rx_tail);

// returns the first unread byte in the recieve buffer
uint8_t read_uart_byte(volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_MAX_PAYLOAD_LENGTH]);