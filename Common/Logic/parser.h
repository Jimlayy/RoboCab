#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "uart_definitions.h"

typedef enum {
    PENDING_HEADER,
    PENDING_LENGTH,
    PARSE_DATABYTE,
    PENDING_CHECKSUM
} uart_state_t;

typedef void (*packet_callback_t)(const uint8_t *data, uint8_t len);

/* All state for ONE port. Give each port its own parser_t. */
typedef struct {
    uart_state_t state;
    uint8_t payload[UART_MAX_PAYLOAD_LENGTH];
    uint8_t length;              // number of data bytes in the packet
    uint8_t index;               // how many we have received so far
    uint8_t checksum;           // are the bytes valid?
    packet_callback_t on_packet; 
} parser_t;

/* Set a parser to its start state. Call once before use. */
void parser_init(parser_t *p, packet_callback_t on_packet);

/* Reset all parser attributes except packet_callback_t on_packet */
void parser_reset(parser_t *p);

/* Feed one received byte to the parser. When a complete packet with a
 * correct footer has arrived, save_packet() is called with its payload.
 * ctx must point to a parser_t (void * so it fits poll_port's callback). */
bool parser_feed(void *ctx, uint8_t byte);

#endif