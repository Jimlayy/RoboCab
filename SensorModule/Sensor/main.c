#include "DistanceSensor.h"
#include "Odometry.h"

#define F_CPU 16000000UL
#define BAUD 115200
#define WHEEL_CIRCUMFERENCE_MM 120      // not measured yet (D-23): prints pulses only
#define STOP_TIMEOUT_MS 1000          // provisional (D-25)

// custom output function
int uart_putchar(char c, FILE *stream) {   // print char to stream (UART)
	// wait for UART to be ready
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
	return 0;
}


// define the stream
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void init_UART() {
    UBRR0H = (unsigned char) (((F_CPU/16.0/BAUD)-1+0.5)/256);
    UBRR0L = (unsigned char) ((F_CPU/16.0/BAUD)-1+0.5); 
    UCSR0B = (1<<TXEN0);                    // Enable TX
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);       // 8-bits,no parity, 1 stop bit
}

int main(void) {
    odometry_sample_t odo;
	uint16_t dist;
    uint32_t last_print = 0;
	
	char val_adc[10];
	char val_odo[10];

    stdout = &mystdout;

    init_UART();
    init_odometry(); // For odometry: Pin 9 on IDC to PD2 and Pin 10 on IDC to PD3
	init_ADC(); // PA2
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