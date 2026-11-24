#include "pwm.h"

const uint16_t dt_reciprocal_LUT[101] PROGMEM = {
    // 0-9 ms
    0,     65535, 32768, 21845, 16384, 13107, 10923, 9362,  8192,  7282,
    // 10-19 ms
    6554,  5958,  5461,  5041,  4681,  4369,  4096,  3855,  3641,  3449,
    // 20-29 ms
    3277,  3121,  2979,  2849,  2731,  2621,  2521,  2427,  2341,  2260,
    // 30-39 ms
    2185,  2114,  2048,  1986,  1928,  1872,  1820,  1771,  1725,  1680,
    // 40-49 ms
    1638,  1598,  1560,  1524,  1489,  1456,  1425,  1394,  1365,  1337,
    // 50-59 ms
    1311,  1285,  1260,  1237,  1214,  1192,  1170,  1150,  1130,  1111,
    // 60-69 ms
    1092,  1074,  1057,  1040,  1024,  1008,  993,   978,   964,   950,
    // 70-79 ms
    936,   923,   910,   898,   886,   874,   862,   851,   840,   830,
    // 80-89 ms
    819,   809,   799,   790,   780,   771,   762,   753,   745,   736,
    // 90-99 ms
    728,   720,   712,   705,   697,   690,   683,   676,   669,   662,
    // 100 ms
    655
};

static MOTOR_SERVO_DATA motor_servo_data = {0};
static PID_DATA pid_data = {0};
static PID_PARAMS pid_params = {0};

void init_motor(){
	DDRB |= (1 << PB3) | (1 << PB2) | (1 << PB1);	// Set PB3 (OC0A) as output for PWM, PB2 for brake and PB1 for direction
	TCCR0A |= (1 << COM0A1) | (1 << WGM00);	// clear OC0A on up, set on down
	TCCR0B |= (1 << CS01) | (1 << CS00);	// prescaler 64  (will generate ~490 Hz signal)    MAX 2KHz
	OCR0A = 0;
}

void init_servo(){
	DDRD |= (1 << PD5); // Set PD5 (OC1A) to output for PWM
	
	// Clear OC1A on comp match, set on BOTTOM
	// Fast PWM, ICR1 := duty cycle
	// prescaler = 8;
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); 	 
		
	ICR1 = SERVO_TOP_50HZ;	// TOP-value (will generate 50 Hz signal)
	motor_servo_data.servo_pulse_width = SERVO_START_PULSE_WIDTH;
	OCR1A = motor_servo_data.servo_pulse_width;
}

void assign_pid_output(int32_t pid_output, uint8_t sensor_mode){
    switch (sensor_mode)
    {
    case LINE_DISTANCE:
        if ((int32_t)OCR1A + pid_output > SERVO_PULSE_WIDTH_MAX){
            OCR1A = SERVO_PULSE_WIDTH_MAX;
        } else if ((int32_t)OCR1A + pid_output < SERVO_PULSE_WIDTH_MIN){
            OCR1A = SERVO_PULSE_WIDTH_MIN;
        } else {
            OCR1A = (int32_t)OCR1A + pid_output;
        }
    break;
    case JUNCTION_DISTANCE:
        if ((int32_t)OCR0A + pid_output > UINT8_MAX){
            OCR1A = UINT8_MAX - 1;  // don't assign full power to be able to change direction (not possible with 100% duty cycle)
        } else if ((int32_t)OCR0A + pid_output < 0){
            OCR1A = 0;
        } else {
            OCR1A = (int32_t)OCR0A + pid_output;
        }
    break;
    
    default:
    break;
    }
}

void run_new_pid(){
    if (pid_data.new_data_is_ready) {
        pid_data.new_data_is_ready = 0;
        
        int16_t error = pid_data.sensor_in - pid_data.wanted_in;
        int16_t delta_error = error - pid_data.prev_error;
        pid_data.prev_error = error;

        int32_t proportional = (int32_t)pid_params.k_p * error;

        if (pid_data.dt > 100){
            pid_data.dt = 100;  // cap maximum dt (shouldn't take this long between readings) just in case
        }

        uint16_t reciprocal_dt = pgm_read_word(&dt_reciprocal_LUT[pid_data.dt]);    // fetch reciprocal version of dt to avoid division
        int32_t raw_derivative = (int32_t)delta_error * reciprocal_dt * pid_params.k_d; // calculate raw derivative term with the reciprocal 
        int32_t derivative = (raw_derivative + (1 << 15)) >> 16;    // add 2^15 for better integer rounding and shift 16 bits right (16 bit reciprocal)
        int32_t output = proportional + derivative;
        printf("Sensor mode: %u --- PD-output: %li\n", pid_data.sensor_mode, output);

        assign_pid_output(output, pid_data.sensor_mode);
    }
}

void set_motor_servo_params(){
	if (motor_servo_data.new_data_is_ready){
		motor_servo_data.new_data_is_ready = 0;
		PORTB = (PORTB & ~((1 << PB2) | (1 << PB1))) | (motor_servo_data.motor_brake << PB2) | (motor_servo_data.motor_dir << PB1);	// store direction in PORTB[1] and brake in PORTB[2]
		OCR0A = motor_servo_data.motor_speed;
		OCR1A = motor_servo_data.servo_pulse_width;
	}
}

void dispatch_data(const uint8_t *payload, uint8_t payload_length){
    printf("--- NEW DATA --- (message id: %u)\n", payload[MESSAGE_ID_IDX]);
    switch (payload[MESSAGE_ID_IDX])
    {   
    case MOTOR_SERVO_MSG_ID:  // motor and servo 
        motor_servo_data.motor_dir = payload[MOTOR_DIR_IDX];
        motor_servo_data.motor_brake = payload[MOTOR_BRAKE_IDX];
        motor_servo_data.motor_speed = payload[MOTOR_SPEED_IDX];

        if (((uint16_t)payload[SERVO_PULSE_WIDTH_HB_IDX] << 8) + payload[SERVO_PULSE_WIDTH_LB_IDX] < SERVO_PULSE_WIDTH_MIN) {
            motor_servo_data.servo_pulse_width = SERVO_PULSE_WIDTH_MIN;
        } else if (((uint16_t)payload[SERVO_PULSE_WIDTH_HB_IDX] << 8) + payload[SERVO_PULSE_WIDTH_LB_IDX] > SERVO_PULSE_WIDTH_MAX){
            motor_servo_data.servo_pulse_width = SERVO_PULSE_WIDTH_MAX;
        } else {
            motor_servo_data.servo_pulse_width = (payload[SERVO_PULSE_WIDTH_HB_IDX] << 8) | payload[SERVO_PULSE_WIDTH_LB_IDX];
        }

        printf("Motor Direction: %u\n", motor_servo_data.motor_dir);
        printf("Motor Brake: %u\n", motor_servo_data.motor_brake);
        printf("Motor Speed: %u\n", motor_servo_data.motor_speed);
        printf("Servo PW: %u\n\n", motor_servo_data.servo_pulse_width);

        motor_servo_data.new_data_is_ready = 1;
        break;
    case PID_DATA_MSG_ID:
        pid_data.dt = payload[DT_IDX];
        pid_data.sensor_in = (payload[SENSOR_IN_H_IDX] << 8) | (payload[SENSOR_IN_L_IDX]);
        pid_data.wanted_in = (payload[WANTED_SENSOR_IN_H_IDX] << 8) | (payload[WANTED_SENSOR_IN_L_IDX]);
        pid_data.sensor_mode = payload[SENSOR_MODE_IDX];
        pid_data.new_data_is_ready = 1;
    break;
    case PID_PARAM_MSG_ID:
        pid_params.k_d = payload[K_D_IDX];
        pid_params.k_p = payload[K_P_IDX];
    break;
    default:
        break;
    }
}