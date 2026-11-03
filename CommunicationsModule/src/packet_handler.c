#include "sensor_data.h"

#define PKT_SENSOR 0x01

// Assemble two bytes into a 16-bit number (little-endian: low byte first)
static uint16_t get_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static int16_t  get_i16(const uint8_t *p) { return (int16_t)get_u16(p); }

//Updates corresponding shared data memory
void handle_packet(const uint8_t *data, uint8_t len)
{
    if (len < 1) return;

    switch (data[0]) {
    case PKT_SENSOR: {
        if (len != 11) {                       // 1 type + 5 × 2 bytes
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