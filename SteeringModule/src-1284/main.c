#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "uart.h"

uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_BUFFER_SIZE];
MOTOR_SERVO_DATA uart_motor_servo_data = {0};
UART_PARSER uart_parser = {0};

int main(void) {
	sei();	// enable global interrupts	
	// motor
	init_motor();

	// servo
	init_servo();

	// init uart with BAUD_RATE
	uart_init(BAUD_RATE);
    while (1) {
		while (uart_read_available(rx_head, rx_tail)){
			uint8_t byte = read_uart_byte(&rx_tail, rx_recieve_buffer);
			process_uart_data(&uart_parser, byte, dispatch_data);
		}

		set_motor_servo_params();

		memset(&uart_parser, 0, sizeof(UART_PARSER));	// reset UART_PARSER
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
	rx_head = (rx_head + 1) % UART_BUFFER_SIZE;
}