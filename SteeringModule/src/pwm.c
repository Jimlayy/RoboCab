#include "pwm.h"

void init_motor(uint8_t pulse_width){
	DDRD |= (1 << PD6) | (1 << PD7);	// Set PD6 (OC0A) as output for PWM and PD7 for direction
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);	// clear OC0A set on up, set on down
	TCCR0B |= (1 << CS01);	// prescaler 8  (will generate ~4kHz signal)
	OCR0A = pulse_width;
}

void set_motor_params(bool direction, uint8_t pulse_width){
	PORTD |= (direction << PD7);
	OCR0A = pulse_width;
}

void init_servo(uint16_t pulse_width){
	DDRB |= (1 << PB1); // Set PB1 (OC1A) to output for PWM
	
	// Clear OC1A on comp match, set on BOTTOM
	// Fast PWM, ICR1 := duty cycle
	// prescaler = 8;
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); 	 
		
	ICR1 = SERVO_TOP_50HZ;	// TOP-value (will generate 50 Hz signal)
	OCR1A = pulse_width; 
}

void set_servo_params(uint8_t pulse_width){
	OCR1A = 2 * pulse_width / SERVO_TOP_50HZ;
}