#include "pwm.h"

void init_motor(){
	DDRB |= (1 << PB3) | (1 << PB2);	// Set PD6 (OC0A) as output for PWM and PD7 for direction
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);	// clear OC0A on up, set on down
	TCCR0B |= (1 << CS01);	// prescaler 8  (will generate ~4kHz signal)
	OCR0A = 0;
}

void set_motor_params(bool direction, uint8_t pulse_width){
	PORTB = (PORTB & ~(1 << PB2)) | (direction << PB2);	// store direction in PORTB[2]
	OCR0A = pulse_width;
}

void init_servo(volatile UART_DATA *uart_data){
	DDRD |= (1 << PD5); // Set PB1 (OC1A) to output for PWM
	
	// Clear OC1A on comp match, set on BOTTOM
	// Fast PWM, ICR1 := duty cycle
	// prescaler = 8;
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); 	 
		
	ICR1 = SERVO_TOP_50HZ;	// TOP-value (will generate 50 Hz signal)
	uart_data->servo_pulse_width = SERVO_START_PULSE_WIDTH;
	OCR1A = uart_data->servo_pulse_width;
}

// uint16_t calculate_servo_pulse_width(uint8_t angle){
// 	return (uint16_t)((((uint32_t)angle * (SERVO_MAX_PULSE_WIDTH - SERVO_MIN_PULSE_WIDTH) * RECIPROCAL_R_ANGLE) >> SHIFT_SCALER_Q_ANGLE) + SERVO_MIN_PULSE_WIDTH);
// }

void set_servo_params(uint16_t pulse_width){
	OCR1A = pulse_width;
}