#include <stdint.h>
#include "parser.h"
#include "protocol.h"
#include "packet_handler.h"
#include "uart_definitions.h"

void parser_init(parser_t *p, packet_callback_t on_packet)
{
    p->state  = PENDING_HEADER;
    p->length = 0;
    p->index  = 0;
    p->checksum = 0;
    p->on_packet = on_packet;
}

// Parses one received byte for the parser given in ctx
void parser_feed(void *ctx, uint8_t byte)
{
    parser_t *p = ctx;

    switch (p->state) {
    case PENDING_HEADER:
        if (byte == UART_HEADER_BYTE)
            p->state = PENDING_LENGTH;
        break;                          // any other byte is ignored

    case PENDING_LENGTH:
        if (byte == 0 || byte > UART_MAX_PAYLOAD_LENGTH) {
            p->state = PENDING_HEADER;  // invalid length: drop the packet
        } else {
            p->length = byte;
            p->index  = 0;
            p->checksum = 0;
            p->state  = PARSE_DATABYTE;
        }
        break;

    case PARSE_DATABYTE:
        p->checksum = p->checksum ^ byte;
        p->payload[p->index++] = byte; 
        if (p->index == p->length)
            p->state = PENDING_CHECKSUM;
        break; 

    case PENDING_CHECKSUM:
        if (p->checksum == byte){
            p->on_packet(p->payload, p->length);}
        p->state = PENDING_HEADER; //Drop the packet if the checksum is not the same  
        break;
    }
}