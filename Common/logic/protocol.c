#include "protocol.h"

void protocol_init(protocol_parser *parser)
{
    parser->state = PENDING_HEADER;
    parser->length = 0;
    parser->received = 0;
}

bool protocol_feed(protocol_parser *parser, uint8_t byte)
{
    switch (parser->state) {
    case PENDING_HEADER:
        if (byte == PROTOCOL_HEADER) {
            parser->state = PENDING_LENGTH;
        }
        break;

    case PENDING_LENGTH:
        if (byte > PROTOCOL_MAX_DATA) {
            parser->state = PENDING_HEADER;
            break;
        }
        parser->length = byte;
        parser->received = 0;
        parser->state = byte == 0 ? PENDING_FOOTER : PARSE_DATABYTE;
        break;

    case PARSE_DATABYTE:
        parser->data[parser->received++] = byte;
        if (parser->received == parser->length) {
            parser->state = PENDING_FOOTER;
        }
        break;

    case PENDING_FOOTER:
        parser->state = PENDING_HEADER;
        return byte == PROTOCOL_FOOTER;
    }

    return false;
}
