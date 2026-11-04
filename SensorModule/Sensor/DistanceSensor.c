/* 
 * File:   EVA-demo.c
 * Author: andni65
 *
 * Created on August 24, 2026, 3:04 PM
 */

// EVA running at 16 MHz external clock
// Hardware:
// UART0
// PA2(ADC2) input for IR-dist


#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#define F_CPU 16000000UL
#define BAUD 115200

void init_UART() {
    UBRR0H = (unsigned char) (((F_CPU/16.0/BAUD)-1+0.5)/256);
    UBRR0L = (unsigned char) ((F_CPU/16.0/BAUD)-1+0.5); 
    UCSR0B = (1<<TXEN0);                    // Enable TX
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);       // 8-bits,no parity, 1 stop bit
}

void init_ADC() {
    ADMUX = (1<<REFS0)|(0<<ADLAR)|(1<<MUX1);// AVCC ref, right adjust, ADC2 input
    ADCSRA = (1<<ADEN)|(7<<ADPS0);          // Enable ADC, prescaler 128
}

int sample_ADC() {
    uint16_t val=0;
    ADCSRA |= (1<<ADSC);                    // start conversion
    while (ADCSRA & (1<<ADSC));             // wait for ADC done
    val = ADCL;                             // read low byte FIRST
    val |= (ADCH<<8);                       // append high byte SECOND
    return val;
}

// custom output function
int uart_putchar(char c, FILE *stream) {   // print char to stream (UART)
    // wait for UART to be ready
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = c;
    return 0;
}

// define the stream
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


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
