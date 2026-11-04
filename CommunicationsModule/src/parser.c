#include <stdint.h>
#include "parser.h"
#include "protocol.h"
#include "packet_handler.h"

// Can currently only handle one thread at a time, so its only possible to poll one port at at time if you
//need to parse data from it



typedef enum {
    PENDING_HEADER,
    PENDING_LENGTH,
    PARSE_DATABYTE,
    PENDING_FOOTER
} uart_state_t;

static uart_state_t state = PENDING_HEADER;
static uint8_t payload[MAX_PAYLOAD];
static uint8_t length;   // number of data bytes in the packet
static uint8_t index_;   // how many we have received so far

// Parses a given byte inside a package
void parser_feed(uint8_t byte)
{
    switch (state) {
    case PENDING_HEADER:
        if (byte == UART_HEADER)
            state = PENDING_LENGTH;
        break;                       // any other byte is ignored

    case PENDING_LENGTH:
        if (byte == 0 || byte > MAX_PAYLOAD) {
            state = PENDING_HEADER;  // invalid length: drop the packet
        } else {
            length = byte;
            index_ = 0;
            state  = PARSE_DATABYTE;
        }
        break;

    case PARSE_DATABYTE:
        payload[index_++] = byte;
        if (index_ == length)
            state = PENDING_FOOTER;
        break;

    case PENDING_FOOTER:
        if (byte == UART_FOOTER)
            save_packet(payload, length);
        // wrong footer: drop it (as the test strategy in the design spec says)
        state = PENDING_HEADER;
        break;
    }
}
