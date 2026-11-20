#include <stdio.h>
#include <time.h>
#include <stdbool.h> 
#include "packet_handler.h"
#include "protocol.h"
#include "port.h"
#include "uart_definitions.h"
#include "sensor_data.h"

// Assemble two bytes into a 16-bit number (little-endian: low byte first)
static uint16_t get_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static int16_t  get_i16(const uint8_t *p) { return (int16_t)get_u16(p); }

// Split a 16-bit number into two bytes (little-endian: low byte first)
static void put_u16(uint8_t *p, uint16_t v) { p[0] = v & 0xFF; p[1] = v >> 8; }

/* ---------- Receiving ---------- */

// Updates corresponding shared data memory
void save_packet(const uint8_t *data, uint8_t len)
{
    if (len < 1) return;

    switch (data[0]) {
    case PKT_SENSOR: {
        if (len != PKT_SENSOR_LEN) {
            fprintf(stderr, "Sensor packet has wrong length: %u\n", len);
            return;
        }
        sensor_data_t s = {0};
        s.distance    = get_u16(&data[1]);
        s.speed_left  = get_u16(&data[3]);             // Not yet decided format
        s.speed_right = get_u16(&data[5]);
        s.gyro_z      = get_i16(&data[7]);
        s.accel_x     = get_i16(&data[9]);
        clock_gettime(CLOCK_MONOTONIC, &s.timestamp);
        sensor_data_update(&s);
        break;
    }
    default:
        fprintf(stderr, "Unknown packet type 0x%02X\n", data[0]);
    }
}

/* ---------- Sending ---------- */

// Wraps the payload in header/length/ and writes it to the port
int send_packet(int fd, const uint8_t *data, uint8_t len)
{
    if (len < 1 || len > UART_MAX_PAYLOAD_LENGTH) {
        fprintf(stderr, "send_packet: invalid length %u\n", len);
        return -1;
    }

    uint8_t frame[UART_MAX_PAYLOAD_LENGTH + 4];
    uint8_t checksum = 0;

    frame[0] = UART_HEADER_BYTE;
    frame[1] = len;
    for (uint8_t i = 0; i < len; i++){
        frame[2 + i] = data[i];
        checksum = checksum ^ data[i];
    }
    frame[2 + len] = checksum;


    printf("[uart] frame (%u):", len + 3);
    for (size_t i = 0; i < (size_t)len + 3; i++)
        printf(" %02X", frame[i]);
    printf("\n");


    return write_all(fd, frame, (size_t)len + 3);
}

// send manual mode steering parameters. Angle is between 1000 and 2000
int send_manual_steering(int fd, bool motor_dir, bool motor_brake, uint8_t speed, uint16_t angle)
{
    printf("start send manual steering");
    uint8_t data[PKT_MAN_STEERING_LEN];
    data[0] = PKT_MAN_STEERING;
    data[1] = motor_dir;
    data[2] = motor_brake;
    data[3] = speed;
    put_u16(&data[4], angle);

    /*  printf("[pkt] bytes (%zu):", sizeof data);
    for (size_t i = 0; i < sizeof data; i++)
        printf(" %02X", data[i]);
    printf("\n");

    return send_packet(fd, data, sizeof data);
    */
}

// send autonomos mode steering parameters.
int send_autonomous_steering(int fd, uint8_t delta_t, uint8_t sensor_mode, uint16_t error)
{
    uint8_t data[PKT_AUT_STEERING_LEN];
    data[0] = PKT_AUT_STEERING;
    data[1] = delta_t;
    data[2] = sensor_mode;
    put_u16(&data[3], error);
    return send_packet(fd, data, sizeof data);
}

int send_PID_params_steering(int fd)
{
    //TODO

    return 0;
}