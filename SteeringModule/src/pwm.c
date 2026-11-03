#include "pwm.h"

void init_motor(){
	DDRD |= (1 << PD6) | (1 << PD7);	// Set PD6 (OC0A) as output for PWM and PD7 for direction
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);	// clear OC0A set on up, set on down
	TCCR0B |= (1 << CS01);	// prescaler 8  (will generate ~4kHz signal)
	OCR0A = 0;
}

void set_motor_params(bool direction, uint8_t pulse_width){
	PORTD = (PORTD & ~(1 << PD7)) | (direction << PD7);
	OCR0A = pulse_width;
}

void init_servo(volatile UART_DATA *uart_data){
	DDRB |= (1 << PB1); // Set PB1 (OC1A) to output for PWM
	
	// Clear OC1A on comp match, set on BOTTOM
	// Fast PWM, ICR1 := duty cycle
	// prescaler = 8;
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); 	 
		
	ICR1 = SERVO_TOP_50HZ;	// TOP-value (will generate 50 Hz signal)
	uart_data->servo_angle = SERVO_START_ANGLE;
	OCR1A = calculate_servo_pulse_width(uart_data->servo_angle); 
}

uint16_t calculate_servo_pulse_width(uint8_t angle){
	return (uint16_t)((((uint32_t)angle * (SERVO_MAX_PULSE_WIDTH - SERVO_MIN_PULSE_WIDTH) * RECIPROCAL_R_ANGLE) >> SHIFT_SCALER_Q_ANGLE) + SERVO_MIN_PULSE_WIDTH);
}

void set_servo_params(uint8_t angle){
	OCR1A = calculate_servo_pulse_width(angle);
}