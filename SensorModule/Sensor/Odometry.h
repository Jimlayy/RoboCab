// Hårdvara:
// PD2 (INT0) left wheel Hall sensor
// PD3 (INT1) right wheel Hall sensor
// Timer0 används som tick varje ms

#ifndef ODOMETRY_HW_H
#define ODOMETRY_HW_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "../logic/odometry.h"

static volatile uint32_t left_pulses, right_pulses; // Pulser sedan start
static volatile uint32_t left_last_ms, right_last_ms; // Tid för senaste puls
static volatile uint32_t ms_counter; // ms sedan start

typedef struct {
    uint32_t left_pulses, right_pulses;
    uint32_t left_last_ms, right_last_ms;
    uint32_t now_ms;
} odometry_sample_t;

void init_odometry() {
    DDRD &= ~((1<<PD2)|(1<<PD3));
    PORTD |= (1<<PD2)|(1<<PD3);
    EICRA = (1<<ISC01)|(1<<ISC11); // INT0, INT1 på falling edge
    EIFR = (1<<INTF0)|(1<<INTF1);
    EIMSK = (1<<INT0)|(1<<INT1);

    TCCR0A = (1<<wgm01);
    TCCR0B = (1<<CS01)|(1<<CS00);
    OCR0A = 249;
    TIMSK0 = (1<<OCIE0A);
}

ISR(TIMER0_COMPA_vect) { ms_counter++; }

ISR(INT0_vect) { left_pulses++; left_last_ms = ms_counter; }
ISR(INT1_vect) { right_pulses++; right_last_ms = ms_counter; }

odometry_sample_t sample_odometry() {
    odometry_sample_t sample;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        sample.left_pulses = left_pulses;
        sample.right_pulses = right_pulses;
        sample.left_last_ms = left_last_ms;
        sample.right_last_ms = right_last_ms;
        sample.now_ms = ms_counter;
    }
    return sample;
}
#endif