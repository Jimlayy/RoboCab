#include <stdio.h>
#include <fcntl.h>
#include <poll.h>



#define UART_HEADER  0xAA   // placeholder: use the values your group agrees on
#define UART_FOOTER  0x55  
#define MAX_PAYLOAD  32

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

// Called when a complete, valid packet has arrived.
static void handle_packet(const uint8_t *data, uint8_t len)
{
    printf("Packet with %u bytes, first byte = 0x%02X\n", len, len ? data[0] : 0);
    // TODO: decode sensor values and update shared memory
}

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
            handle_packet(payload, length);
        // wrong footer: drop it (as the test strategy in the design spec says)
        state = PENDING_HEADER;
        break;
    }
}

int main(void)
{
    // Öppnar filen för USB-kontakten som read only
    int usb = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY)

    struct pollfd pfd;
    pfd.fd      = fd;        // the descriptor from open()
    pfd.events  = POLLIN;    // what you want: "tell me when there is data to read"
    pfd.revents = 0;         // the kernel fills this in; you don't need to set it

    while (running) {
    int r = poll(&pfd, 1, 500);

    if (r < 0) {
        if (errno == EINTR) continue;      // interrupted by a signal, try again
        perror("poll");
        break;
    }
    if (r == 0) {
        fprintf(stderr, "No data from the sensor module for 500 ms\n");
        continue;                          // timeout: just wait again
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        fprintf(stderr, "Port lost (cable unplugged?)\n");
        break;
    }
    if (pfd.revents & POLLIN) {
        uint8_t buf[64];
        ssize_t n = read(fd, buf, sizeof buf);
        for (ssize_t i = 0; i < n; i++)
            parser_feed(buf[i]);
    }
}
close(fd);

    printf("Hello RoboCab!\n");
    return 0;        // 0 = program finished OK
}