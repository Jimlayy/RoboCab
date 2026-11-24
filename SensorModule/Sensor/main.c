#include "DistanceSensor.h"
#include "Odometry.h"
#include "IMU.h"
#include "protocol.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define WHEEL_CIRCUMFERENCE_MM 120      // not measured yet (D-23): prints pulses only
#define STOP_TIMEOUT_MS 1000          // provisional (D-25)
#define DEBUG false					// Readable data in tera term

// custom output function
int uart_putchar(char c, FILE *stream) {   // print char to stream (UART)
	// wait for UART to be ready
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
	return 0;
}

// define the stream
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

#define PACKET_HEADER       0xAA
#define PAYLOAD_LENGTH      24
#define PACKET_LENGTH       27

// Add a 32-bit value to the payload, least significant byte first.
void append_u32(uint8_t *payload, uint8_t *index, uint32_t value)
{
	payload[(*index)++] = (uint8_t)(value);
	payload[(*index)++] = (uint8_t)(value >> 8);
	payload[(*index)++] = (uint8_t)(value >> 16);
	payload[(*index)++] = (uint8_t)(value >> 24);
}

// Add a 16-bit value to the payload, least significant byte first.
void append_u16(uint8_t *payload, uint8_t *index, int16_t value)
{
	uint16_t data = (uint16_t)value;

	payload[(*index)++] = (uint8_t)(data);
	payload[(*index)++] = (uint8_t)(data >> 8);
}

// Construct and transmit the complete packet.
void send_packet(odometry_sample_t odo, uint16_t dist, imu_sample_t imu)
{
	uint8_t packet[PACKET_LENGTH];
	uint8_t index = 0;
	uint8_t checksum = 0;

	int16_t accel_x = (int16_t)imu_accel_mg(imu.ax);
	int16_t gyro_z  = (int16_t)imu_gyro_mdps(imu.gz);

	// Header and payload length
	packet[index++] = PACKET_HEADER;
	packet[index++] = PAYLOAD_LENGTH;

	// packet identifier
	packet[index++] = (uint8_t)PKT_SENSOR

	// distance
	append_u16(packet, &index, (int16_t)dist);

	// odometry
	append_u32(packet, &index, odo.left_pulses);
	append_u32(packet, &index, odo.right_pulses);
	append_u32(packet, &index, odo.left_tslp);
	append_u32(packet, &index, odo.right_tslp);

	// IMU
	append_u16(packet, &index, accel_x);
	append_u16(packet, &index, gyro_z);

	// IMU ok
	packet[index++] = (uint8_t)imu.ok;

	// checksum
	for (uint8_t i = 2; i < index; i++)
	{
		checksum ^= packet[i];
	}

	// Append checksum
	packet[index++] = checksum;

	// send data
	for (uint8_t i = 0; i < index; i++)
	{
		printf("%c", (int)packet[i]);
	}
}

void init_UART() {
    UBRR0H = (unsigned char) (((F_CPU/16.0/BAUD)-1+0.5)/256);
    UBRR0L = (unsigned char) ((F_CPU/16.0/BAUD)-1+0.5); 
    UCSR0B = (1<<TXEN0);                    // Enable TX
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);       // 8-bits,no parity, 1 stop bit
}

int main(void) {
    odometry_sample_t odo;
	uint16_t dist;
	imu_sample_t imu;
	
	char val_adc[10];

    stdout = &mystdout;

    init_UART();
    init_odometry(); // For odometry: Pin 9 on IDC to PD2 and Pin 10 on IDC to PD3
	init_ADC(); // PA2
	init_IMU();
	
    sei();

    while(1) {
        dist = sample_ADC();
		odo = sample_odometry();
		imu = sample_IMU();
		
		if(DEBUG) {
			itoa(dist, val_adc, 10);
			printf("Distance: %s\n", val_adc);	
			printf("Right Time since last pulse: %lu\n", (unsigned long)odo.right_tslp);
			printf("Left Time since last pulse: %lu\n", (unsigned long)odo.left_tslp);
			printf("Acceleration %d\n", imu_accel_mg(imu.ax)); // 2 bytes
			printf("Gyro: %d\n", imu_gyro_mdps(imu.gz)); // 2 bytes
			printf("Right pulses: %lu\n", (unsigned long)odo.right_pulses);
			printf("Left pulses: %lu\n", (unsigned long)odo.left_pulses);
		}
		else {
			send_packet(odo, dist, imu);
		}
	
	
		/*	What we need to send:
		*	HEADER (2bytes)
		*	Left and right pulses (4 bytes each, unsigned)
		*	Left and right tslp (4 bytes each, unsigned)
		*	Acceleration in x axis (2 bytes)
		*	Rotation in z axis (2 bytes)
		*	Distance (2 bytes)
		*	IMU.ok (1 byte)
		*/
		
	}
}

//TODO: Implement IMU_ok bool at the end of the payload
//If possible optimize payload