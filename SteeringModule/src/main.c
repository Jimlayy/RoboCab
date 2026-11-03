#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "uart.h"
#include "lcd_helper_functions.h"

volatile uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_BUFFER_SIZE];
volatile UART_DATA uart_data = {0};

int main(void) {
	sei();	// enable global interrupts	
	// motor
	init_motor();

	// servo
	init_servo(&uart_data);

	// init uart with BAUD_RATE
	uart_init(BAUD_RATE);
    while (1) {
		process_uart_data(rx_head, &rx_tail, rx_recieve_buffer, &uart_data);
		set_motor_params(uart_data.motor_dir, uart_data.motor_speed);
		set_servo_params(uart_data.servo_angle);
		PORTB |= (uint8_t)uart_data.state << 5;
		PORTB &= ~(0xE0);
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
	PORTB |= (1 << PB4);
	PORTB &= ~(1 << PB4);
}