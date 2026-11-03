#include "twi.h"
#include "definitions.h"

uint8_t twi_start_cond(void){
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	// send START cond.
	while (!(TWCR & (1 << TWINT))){}		// wait for TWINT to be pulled HIGH (START cond. transmitted)
	if ((TWSR & PRESCALER_MASK) != TWI_START_ACK){
		return 1;
	}
	return 0;
}

uint8_t twi_repeat_start_cond(void){
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);	// send repeated start cond
	while (!(TWCR & (1 << TWINT))){}		// wait for TWINT to be pulled HIGH (SLA+W cond. transmitted)
	if ((TWSR & PRESCALER_MASK) != TWI_REP_START_ACK){	
		return 1;
	}
	return 0;
}

uint8_t twi_slaw_cond(uint8_t slave_address){
	TWDR = (slave_address << 1);	// send SLA+W 
	TWCR = (1<<TWINT) | (1<<TWEN);	// clear TWINT flag
	while (!(TWCR & (1 << TWINT))){}		// wait for TWINT to be pulled HIGH (SLA+W cond. transmitted)
	if ((TWSR & PRESCALER_MASK) != TWI_SLAW_ACK){	
		return 1;
	}
	return 0;
}

uint8_t twi_retry_loop(uint8_t slave_address){
	uint8_t return_status = 0;
	uint8_t retries = 0;
	while (retries < MAX_RETRIES){
		twi_repeat_start_cond();
		return_status = twi_slaw_cond(slave_address);
		if (return_status == 0) return return_status;
		retries++;
	}
	return return_status;
}

uint8_t twi_send_bytes(uint8_t slave_address, const uint8_t *twi_data, uint8_t bytes_to_send){
	volatile uint8_t return_status;
	TWBR = 198;	// set bit rate
	TWSR |= 0x01; // prescaler = 4
	
	return_status = twi_start_cond();
	if (return_status != 0){
		TWCR = (1<<TWINT) | (1 <<TWSTO) | (1<<TWEN); // send STOP cond. to free I2C bus	
		return return_status;
	}
	
	return_status = twi_slaw_cond(slave_address);
	if (return_status != 0){
		return_status = twi_retry_loop(slave_address);
		if (return_status != 0) return return_status;
	}
	
	for (int i = 0; i < bytes_to_send; i++){
		TWDR = twi_data[i];		// transmit data byte
		TWCR = (1<<TWINT) | (1<<TWEN);	// clear TWINT flag to cont. with transmission
		while (!(TWCR & (1 << TWINT))){}		// wait for transmission to complete
		
		if ((TWSR & PRESCALER_MASK) != TWI_DATA_TRANSMIT_ACK){
			TWCR = (1<<TWINT) | (1 <<TWSTO) | (1<<TWEN); // send STOP cond. to free I2C bus
			return 1;
		}
	}
	
	TWCR = (1<<TWINT) | (1 <<TWSTO) | (1<<TWEN); // send STOP cond.
	
	return 0;
}