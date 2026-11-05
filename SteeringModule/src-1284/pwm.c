#include "pwm.h"

static MOTOR_SERVO_DATA motor_servo_data = {0};
static PID_DATA pid_data = {0};

void init_motor(){
	DDRB |= (1 << PB3) | (1 << PB2);	// Set PD6 (OC0A) as output for PWM and PD7 for direction
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);	// clear OC0A on up, set on down
	TCCR0B |= (1 << CS01);	// prescaler 8  (will generate ~4kHz signal)
	OCR0A = 0;
}

void init_servo(){
	DDRB |= (1 << PB1); // Set PB1 (OC1A) to output for PWM
	
	// Clear OC1A on comp match, set on BOTTOM
	// Fast PWM, ICR1 := duty cycle
	// prescaler = 8;
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); 	 
		
	ICR1 = SERVO_TOP_50HZ;	// TOP-value (will generate 50 Hz signal)
	motor_servo_data.servo_pulse_width = SERVO_START_PULSE_WIDTH;
	OCR1A = motor_servo_data.servo_pulse_width;
}

void set_motor_servo_params(){
	if (motor_servo_data.new_data_is_ready){
		motor_servo_data.new_data_is_ready = 0;
		PORTB = (PORTB & ~((1 << PB2) | (1 << PB1))) | (motor_servo_data.motor_dir << PB2) | (motor_servo_data.motor_brake << PB1);	// store direction in PORTB[2] and brake in PORTB[1]
		OCR0A = motor_servo_data.motor_speed;
		OCR1A = motor_servo_data.servo_pulse_width;
	}
}

void dispatch_data(const uint8_t *payload, uint8_t payload_length){
    switch (payload[MESSAGE_ID_IDX])
    {   
    case 0x01:  // motor and servo 
    {
        motor_servo_data.motor_dir = payload[MOTOR_DIR_IDX];
        motor_servo_data.motor_brake = payload[MOTOR_BRAKE_IDX];
        motor_servo_data.motor_speed = payload[MOTOR_SPEED_IDX];
        motor_servo_data.servo_pulse_width = (payload[SERVO_PULSE_WIDTH_HB_IDX] << 8) | payload[SERVO_PULSE_WIDTH_LB_IDX];
        motor_servo_data.new_data_is_ready = 1;
        break;
    }
    case 0x02:
    {
        pid_data.k_p = payload[K_P_IDX];
        pid_data.k_d = payload[K_D_IDX];
        pid_data.dt = payload[DT_IDX];
        pid_data.sensor_input = payload[SENSOR_INPUT_IDX];
        pid_data.wanted_input = payload[WANTED_INPUT_IDX];
        pid_data.new_data_is_ready = 1;
    break;
    }

    default:
        break;
    }
}