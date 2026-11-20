#include "definitions.h"

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void init_custom_stdout(){
    stdout = &mystdout;
}

int uart_putchar(char c, FILE *stream) {   // print char to stream (UART)
    // wait for UART to be ready
    while (!(UCSR1A & (1<<UDRE1)));
    UDR1 = c;
    return 0;
}