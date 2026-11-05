#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <stdint.h>

/* --- Receiving --- */

/* Decode a complete packet (without header, length and footer) and store
 * the values in the corresponding shared memory. data[0] is the packet type. */
void save_packet(const uint8_t *data, uint8_t len);

/* --- Sending --- */

/* Wrap data in [HEADER][len][data...][FOOTER] and write it to fd.
 * data[0] must be the packet type. Returns 0 on success, -1 on error. */
int send_packet(int fd, const uint8_t *data, uint8_t len);

/* Send speed and steering angle to the steering module.
 * Returns 0 on success, -1 on error. */
int send_steering(int fd, int16_t speed, int16_t angle);

#endif