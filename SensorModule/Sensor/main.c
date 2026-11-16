#include "DistanceSensor.h"
#include "Odometry.h"

#define WHEEL_CIRCUMFERENCE_MM 120      // not measured yet (D-23): prints pulses only
#define STOP_TIMEOUT_MS 1000          // provisional (D-25)

int main(void) {
    odometry_sample_t odo;
	uint16_t dist;
    uint32_t last_print = 0;
	
	char val_adc[10];
	char val_odo[10];

    stdout = &mystdout;

    init_UART();
    init_odometry(); // For odometry: Pin 9 on IDC to PD2 and Pin 10 on IDC to PD3
	init_ADC();
    sei();

    while(1) {
        dist = sample_ADC();
		odo = sample_odometry();
		
		itoa(dist, val_adc, 10);
		
		printf("Right Time since last pulse: %lu\n", (unsigned long)odo.right_tslp);
		printf("Left Time since last pulse: %lu\n", (unsigned long)odo.left_tslp);
		//printf("Right pulses: %lu\n", (unsigned long)odo.right_pulses);
		//printf("Left pulses: %lu\n", (unsigned long)odo.left_pulses);
		//printf("%s\n", val_adc);
    }
}