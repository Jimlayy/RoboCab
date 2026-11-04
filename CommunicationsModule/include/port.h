#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <stddef.h>
#include <signal.h>

/* Open a serial port in raw 8N1 mode. Returns fd, or -1 on error. */
int open_port(const char *dev, int baud);

/* Read from fd and feed every byte to on_byte() until *running becomes 0
 * or the port fails. Returns 0 on normal stop, -1 on error. */
int poll_port(int fd, volatile sig_atomic_t *running, void (*on_byte)(uint8_t));

/* Write all n bytes to fd. Returns 0 on success, -1 on error. */
int write_all(int fd, const uint8_t *buf, size_t n);

#endif