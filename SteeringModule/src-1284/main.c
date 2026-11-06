#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "uart.h"
#include "parser.h"

uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_MAX_PAYLOAD_LENGTH];
parser_t uart_parser;
bool full_message_assigned = 0;

int main(void) {
	sei();	// enable global interrupts	
	parser_init(&uart_parser, dispatch_data);

	// motor
	init_motor();

	// servo
	init_servo();

	// init uart with BAUD_RATE
	uart_init();
    while (1) {
		while (uart_read_available(rx_head, rx_tail)){
			uint8_t byte = read_uart_byte(&rx_tail, rx_recieve_buffer);
			full_message_assigned = parser_feed(&uart_parser, byte);
		}

		if (full_message_assigned) {
			set_motor_servo_params();
		}
	}
}

// UART recieve buffer full interrupt
ISR(USART0_RX_vect){
	uint8_t status = UCSR0A;
	if ((status & 0x1C) != 0){	// if error detected
		(void)UDR0;
		return;
	}

	rx_recieve_buffer[rx_head] = UDR0;
	rx_head = (rx_head + 1) % UART_MAX_PAYLOAD_LENGTH;
}