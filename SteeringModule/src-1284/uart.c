#include "uart.h"

void uart_init(){
    UBRR1H = (unsigned char) (((F_CPU/16.0/BAUD_RATE)-1+0.5)/256);
    UBRR1L = (unsigned char) ((F_CPU/16.0/BAUD_RATE)-1+0.5); 

    /*Set baud rate */
    UBRR0H = (MYUBBR>>8);
    UBRR0L = MYUBBR;

    cli();
    UCSR1B = (1<<TXEN1);                    // Enable TX
    UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);       // 8-bits,no parity, 1 stop bit

    /*Enable receiver and reciever interrupt */
    UCSR0A = (1 << U2X0);
    UCSR0B = (1<<RXCIE0)|(1<<RXEN0);
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (3<<UCSZ00);
    sei();
}

uint8_t uart_read_available(uint8_t rx_head, uint8_t rx_tail){
    return (rx_tail != rx_head);
}

uint8_t read_uart_byte(volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_MAX_PAYLOAD_LENGTH]){
    uint8_t data = rx_buffer[*rx_tail];
    *rx_tail = (*rx_tail + 1) % UART_MAX_PAYLOAD_LENGTH; // calculate new tail
    return data;
}