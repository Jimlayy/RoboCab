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



