#include "parser.h"

void parser_init(parser_t *p, packet_callback_t on_packet)
{
    p->state  = PENDING_HEADER;
    p->length = 0;
    p->index  = 0;
    p->checksum = 0;
    p->on_packet = on_packet;
}

void parser_reset(parser_t *p){
    p->index  = 0;
    p->checksum = 0;
}

// Parses one received byte for the parser given in ctx
int16_t parser_feed(void *ctx, uint8_t byte)
{
    int16_t return_status = -1;
    parser_t *p = ctx;

    switch (p->state) {
    case PENDING_HEADER:
        if (byte == UART_HEADER_BYTE){
            p->state = PENDING_LENGTH;
            return_status = SUCCESSFUL_BYTE;
        }
        break;                          // any other byte is ignored

    case PENDING_LENGTH:
        if (byte == 0 || byte > UART_MAX_PAYLOAD_LENGTH) {
            p->state = PENDING_HEADER;  // invalid length: drop the packet
        } else {
            parser_reset(p);
            p->length = byte;
            p->state  = PARSE_DATABYTE;
            return_status = SUCCESSFUL_BYTE;
        }
        break;

    case PARSE_DATABYTE:
        p->checksum = p->checksum ^ byte;
        p->payload[p->index++] = byte; 
        return_status = SUCCESSFUL_BYTE;
        if (p->index == p->length)
            p->state = PENDING_CHECKSUM;
        break; 

    case PENDING_CHECKSUM:
        p->state = PENDING_HEADER; //Drop the packet if the checksum is not the same  
        if (p->checksum == byte){
            p->on_packet(p->payload, p->length);
            return_status = p->payload[0];  // return message id when succesfull packet has been sent
        }
    }
    return return_status;
}