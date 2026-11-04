#include "DistanceSensor.h"

int main(void) {
	int val;
	char str[10];
	
	stdout = &mystdout;
	
	init_UART();
	init_ADC();
	
	while(1) {
		val = sample_ADC();
		
		itoa(val, str, 10);
		
		printf("%s\n", str);
		
	}
}