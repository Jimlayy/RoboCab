#include "uart.h"
#include "definitions.h"

void uart_init(uint16_t baud_rate){
    uint16_t myubbr = FOSC/16/baud_rate-1;
    /*Set baud rate */
    UBRR0H = (myubbr>>8);
    UBRR0L = myubbr;

    cli();
    /*Enable receiver and reciever interrupt */
    UCSR0B = (1<<RXCIE0)|(1<<RXEN0);
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (3<<UCSZ00);
    sei();
}

uint8_t uart_read_available(uint8_t rx_head, uint8_t rx_tail){
    return (rx_tail != rx_head);
}

uint8_t read_uart_byte(volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_BUFFER_SIZE]){
    uint8_t data = rx_buffer[*rx_tail];
    *rx_tail = (*rx_tail + 1) % UART_BUFFER_SIZE; // calculate new tail
    return data;
}

void process_uart_data(uint8_t rx_head, volatile uint8_t *rx_tail, volatile uint8_t rx_buffer[UART_BUFFER_SIZE], volatile UART_DATA *uart_data){
    while (uart_read_available(rx_head, *rx_tail)) {
        uint8_t msg = read_uart_byte(rx_tail, rx_buffer);

        switch (uart_data->state)
        {
            case PENDING_HEADER:
                uart_data->checksum = 0;
                if (msg == UART_HEADER_BYTE){
                    uart_data->state = UART_MOTOR_DIR;
                }
            break;

            case UART_MOTOR_DIR:
                uart_data->checksum ^= msg;
                uart_data->temp_motor_dir = msg;
                uart_data->state = UART_MOTOR_SPEED;
            break;

            case UART_MOTOR_SPEED:
                uart_data->checksum ^= msg;
                uart_data->temp_motor_speed = msg;
                uart_data->state = UART_SERVO_ANGLE;
            break;

            case UART_SERVO_ANGLE:
                uart_data->checksum ^= msg;
                uart_data->temp_servo_angle = msg;
                uart_data->state = PENDING_FOOTER;
            break;

            case PENDING_FOOTER:
                if (uart_data->checksum == msg){
                    uart_data->motor_dir = uart_data->temp_motor_dir;
                    uart_data->motor_speed = uart_data->temp_motor_speed;
                    uart_data->servo_angle = uart_data->temp_servo_angle;
                    PORTD |= (1 << PD5);
                }else{
                    PORTD &= ~(1 << PD5);
                }

                uart_data->state = PENDING_HEADER;
            break;

        default:
            uart_data->state = PENDING_HEADER;
            break;
        }
    }
}