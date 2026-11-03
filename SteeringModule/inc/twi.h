#include <avr/io.h>
#include <avr/interrupt.h>

// Send start condition over I2C bus
uint8_t twi_start_cond(void);

// Send repeated start condition over I2C bus
uint8_t twi_repeat_start_cond(void);

// Send a slave write condition to (slave_address)
uint8_t twi_slaw_cond(uint8_t slave_address);

// Retry sending SLA+W conditon to (slave_address), maximum MAX_RETRIES retries
uint8_t twi_retry_loop(uint8_t slave_address);

// Send (bytes_to_send) bytes to (slave_address)
uint8_t twi_send_bytes(uint8_t slave_address, const uint8_t *twi_data, uint8_t bytes_to_send);