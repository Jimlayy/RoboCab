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

void dispatch_data(const uint8_t *payload, uint8_t payload_length){
    switch (payload[MESSAGE_ID_IDX])
    {   
    case 0x01:  // motor and servo 
    {
        UART_MOTOR_SERVO_DATA motor_servo_data = {0};
        motor_servo_data.motor_dir = payload[MOTOR_DIR_IDX];
        motor_servo_data.motor_brake = payload[MOTOR_BRAKE_IDX];
        motor_servo_data.motor_speed = payload[MOTOR_SPEED_IDX];
        motor_servo_data.servo_pulse_width = (payload[SERVO_PULSE_WIDTH_HB_IDX] << 8) | payload[SERVO_PULSE_WIDTH_LB_IDX];
        motor_servo_data.new_data_is_ready = 1;
        break;
    }
    case 0x02:
    {
        UART_PID_DATA pid_data = {0};
        pid_data.k_p = payload[K_P_IDX];
        pid_data.k_d = payload[K_D_IDX];
        pid_data.dt = payload[DT_IDX];
        pid_data.sensor_input = payload[SENSOR_INPUT_IDX];
        pid_data.wanted_input = payload[WANTED_INPUT_IDX];
        pid_data.new_data_is_ready = 1;
    break;
    }

    default:
        break;
    }
}

void process_uart_data(void *context, uint8_t byte, void (*function_callback_t)(uint8_t*, uint8_t)){
    UART_PARSER *parser = context;

    switch (parser->state)
    {
        case PENDING_HEADER:
            if (byte == UART_HEADER_BYTE) {
                parser->state = READ_LENGTH;
            }
        break;

        case READ_LENGTH:
            parser->checksum ^= byte;
            if (byte > UART_BUFFER_SIZE) {
                parser->state = PENDING_HEADER;
            }else{
                parser->expected_length = byte;
                parser->state = READ_PAYLOAD;   
            }
        break;

        case READ_PAYLOAD:
            parser->temp_rx_buffer[parser->payload_index] = byte;
            parser->payload_index++;
            if (parser->payload_index == parser->expected_length) {
                parser->state = PENDING_FOOTER;
            }
            parser->checksum ^= byte;
        break;

        case PENDING_FOOTER:
            if (parser->checksum == byte) {
                dispatch_data(parser->temp_rx_buffer, parser->expected_length);
            }
            
            parser->state = PENDING_HEADER;
        break;

    default:
        parser->state = PENDING_HEADER;
        break;
    }
}