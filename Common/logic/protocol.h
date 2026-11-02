#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

/* Paketformat på UART: header, längd, data, footer (designspec 3.6.3, 7.2.1). */

/* Designspecen anger inte värdena. Dessa är preliminära tills gruppen beslutat. */
#define PROTOCOL_HEADER 0xAA
#define PROTOCOL_FOOTER 0x55
#define PROTOCOL_MAX_DATA 16

typedef enum {
    PENDING_HEADER,
    PENDING_LENGTH,
    PARSE_DATABYTE,
    PENDING_FOOTER
} UART_STATES;

typedef struct {
    UART_STATES state;
    uint8_t length;
    uint8_t received;
    uint8_t data[PROTOCOL_MAX_DATA];
} protocol_parser;

void protocol_init(protocol_parser *parser);

/* Tar emot en byte. Returnerar true när ett helt giltigt paket finns i
   parser->data och parser->length, och är då giltigt till nästa anrop. */
bool protocol_feed(protocol_parser *parser, uint8_t byte);

#endif
