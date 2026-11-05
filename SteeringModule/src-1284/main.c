#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "uart.h"

uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_BUFFER_SIZE];
UART_DATA uart_data = {0};
UART_PARSER uart_parser = {0};

int main(void) {
	sei();	// enable global interrupts	
	// motor
	init_motor();

	// servo
	init_servo(&uart_data);

	// init uart with BAUD_RATE
	uart_init(BAUD_RATE);
    while (1) {
		while (uart_read_available(rx_head, rx_tail)){
			uint8_t byte = read_uart_byte(rx_tail, rx_recieve_buffer);
			process_uart_data(&uart_parser, byte);
		}
		set_motor_params(uart_data.motor_dir, uart_data.motor_speed);
		set_servo_params(uart_data.servo_pulse_width);	
		memset(&uart_parser, 0, sizeof(UART_PARSER));	// reset UART_PARSER
	}
}

// UART recieve buffer full interrupt
ISR(USART_RX_vect){
	uint8_t status = UCSR0A;
	if ((status & 0x1C) != 0){	// if error detected
		(void)UDR0;
		return;
	}

	rx_recieve_buffer[rx_head] = UDR0;
	rx_head = (rx_head + 1) % UART_BUFFER_SIZE;
}