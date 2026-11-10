#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "uart.h"
#include "parser.h"

uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_MAX_PAYLOAD_LENGTH];
parser_t uart_parser;
uint8_t message_id = 0;

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
			message_id = parser_feed(&uart_parser, byte);
		}

		if (message_id != 0) {
			switch (message_id)
			{
			case MOTOR_SERVO_MSG_ID:
				set_motor_servo_params();
				break;
			case PID_DATA_MSG_ID:
				calculate_new_pid_output();
			break;
			default:
				break;
			}
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

	uint8_t buf = rx_recieve_buffer[rx_head];
	PORTD = (PORTD & ~(0b01011000));
    bool bit0 = buf & 0x01;
    bool bit1 = buf & 0x02;
    bool bit2 = buf & 0x04;
	PORTD |= (bit0 << PD3) | (bit1 << PD4) | (bit2 << PD6);

	rx_head = (rx_head + 1) % UART_MAX_PAYLOAD_LENGTH;
}