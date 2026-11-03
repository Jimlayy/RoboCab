#include "lcd_helper_functions.h"
#include "twi.h"
#include "definitions.h"

void lcd_print(const char *str) {
    uint8_t char_packet[2];
    char_packet[0] = I2C_DATA_COMMAND; // Control Byte
    
    while (*str) {
        char_packet[1] = *str; // Load the current character
        
        // Send the 2-byte packet
        twi_send_bytes(LCD_I2C_ADDRESS, char_packet, 2);
        
        str++;
    }
}

void lcd_clear() {
	uint8_t char_packet[] = {I2C_INSTRUCTION_COMMAND, 0x01};
	twi_send_bytes(LCD_I2C_ADDRESS, char_packet, ARRAY_SIZE(char_packet));
}