/*
 * uart_port.h - Opening and reading/writing the USB serial ports.
 *
 * WHY THIS EXISTS
 * The sensor and steering modules (ATmega1284) are connected to the
 * Raspberry Pi through TTL-232R USB cables, which appear in Linux as
 * serial devices (e.g. /dev/ttyUSB0). Before any bytes can be exchanged
 * the port must be opened and configured: baud rate, 8 data bits, no
 * parity, raw mode (no line editing or echo).
 *
 * MODULARITY
 * This component only moves raw bytes. It knows nothing about packets or
 * what the bytes mean - that is the job of uart_parser and uart_encoder.
 * Keeping the Linux-specific port handling here means the rest of the
 * module would not change if the physical link changed (for example to
 * the Pi's GPIO UART), and test programs can feed bytes into the parser
 * without a real port.
 */
#ifndef UART_PORT_H
#define UART_PORT_H

#include <stddef.h>
#include <sys/types.h>

/* Open and configure a serial device. Returns a file descriptor or -1. */
int uart_port_open(const char *device, int baud);

/* Read the bytes currently available. Returns count, 0 if none, -1 on error. */
ssize_t uart_port_read(int fd, void *buf, size_t len);

/* Write all bytes. Returns 0 on success, -1 on error. */
int uart_port_write(int fd, const void *buf, size_t len);

void uart_port_close(int fd);

#endif /* UART_PORT_H */
