#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

/* Feed one received byte to the UART packet parser.
 * When a complete packet with a correct footer has arrived,
 * save_packet() is called with the payload (type byte + data).
 *
 * Note: the parser has a single global state, so only ONE port
 * may feed it. Bytes from two ports would mix into broken packets. */
void parser_feed(uint8_t byte);

#endif