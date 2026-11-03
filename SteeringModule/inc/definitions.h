#include <stdbool.h>

//UART
#define FOSC 16000000UL // Clock Speed
#define BAUD_RATE 9600
#define UART_BUFFER_SIZE 64
#define UART_HEADER_BYTE 0xA0

//PWM
#define SERVO_TOP_50HZ 39999

// TWI
#define TWI_START_ACK 0x08
#define TWI_REP_START_ACK 0x10
#define TWI_SLAW_ACK 0x18
#define TWI_SLAW_NACK 0x20
#define TWI_DATA_TRANSMIT_ACK 0x28
#define PRESCALER_MASK 0xF8
#define MAX_RETRIES 10

// I2C
#define I2C_INSTRUCTION_COMMAND 0x00
#define I2C_DATA_COMMAND 0x40
#define LCD_I2C_ADDRESS 0x3E

// Other
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))