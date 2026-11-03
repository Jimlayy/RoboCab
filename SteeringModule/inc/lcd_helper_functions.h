#include <avr/io.h>
#include <avr/interrupt.h>

// Print a string to the display
void lcd_print(const char *str);

// Clear the screen
void lcd_clear(void);