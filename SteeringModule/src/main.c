#include <avr/io.h>
#include <avr/interrupt.h>
#include "twi.h"
#include "pwm.h"
#include "uart.h"
#include "lcd_helper_functions.h"

volatile uint8_t rx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_recieve_buffer[UART_BUFFER_SIZE];
volatile UART_DATA uart_data = {0};

int main(void) {
	sei();	// enable global interrupts
	
	DDRB |= 0xF0; 
	
	EICRA |= 0b00001111;		// rising edge INT0 and INT1 ext. int.
	EIMSK |= (1 << INT1) | (1 << INT0);	// enabling ext. int. on INT1 and INT0
	
	// motor
	uint8_t duty = 128;
	init_motor(duty);

	// servo
	uint16_t pulse_width = 3000; // 1.5 ms
	init_servo(pulse_width);

	// init uart with BAUD_RATE
	uart_init(BAUD_RATE);
    while (1) {
		process_uart_data(rx_head, &rx_tail, rx_recieve_buffer, &uart_data);
		set_motor_params(uart_data.motor_dir, uart_data.motor_speed);
		PORTB = (uint8_t)uart_data.state << 5;
		PORTB = 0;
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