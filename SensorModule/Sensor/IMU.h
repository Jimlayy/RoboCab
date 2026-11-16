#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int16_t ax, ay, az;  // Accelerometer
    int16_t gx, gy, gz;  // Gyroscope
    int16_t mx, my, mz;  // Magnetometer
} IMU_data;

void init_IMU() {
	DDRC &= ~((1<<PC0)|(1<<PC1));
    PORTC |= (1<<PC0)|(1<<PC1);
}