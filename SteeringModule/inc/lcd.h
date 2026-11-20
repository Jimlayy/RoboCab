#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#include "definitions.h"

void wait_for_low_busy_flag();

void write_lcd_data(bool data_type, uint8_t data);

void init_lcd();

void update_lcd();