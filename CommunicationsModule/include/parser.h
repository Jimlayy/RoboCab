#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "protocol.h"

typedef enum {
    PENDING_HEADER,
    PENDING_LENGTH,
    PARSE_DATABYTE,
    PENDING_CHECKSUM,
    PENDING_FOOTER
} uart_state_t;

/* All state for ONE port. Give each port its own parser_t. */
typedef struct {
    uart_state_t state;
    uint8_t payload[MAX_PAYLOAD];
    uint8_t length;              // number of data bytes in the packet
    uint8_t index;               // how many we have received so far
    const char *name;            // e.g. "sensor", for debug prints
    uint8_t checksum;           // are the bytes valid?
} parser_t;

/* Set a parser to its start state. Call once before use. */
void parser_init(parser_t *p, const char *name);

/* Feed one received byte to the parser. When a complete packet with a
 * correct footer has arrived, save_packet() is called with its payload.
 * ctx must point to a parser_t (void * so it fits poll_port's callback). */
void parser_feed(void *ctx, uint8_t byte);

#endif