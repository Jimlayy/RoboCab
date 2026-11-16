#include "lcd.h"

/*
Read (R/W = 1), Write (R/W = 0)
Busy flag read from PB7

reset init:
    - Clear display
    - 8-bit interface
    - 1 line display (N = 0, change later to 1)
    - 5x8 dot display
    - Display off (D = 0, change later to 1)
    - Cursor off
    - Blinking off
    - Inc by 1
    - No shift
*/

void wait_for_low_busy_flag(){
    bool busy_flag = 1;
    DDRA &= 0;   // input (not accidentally put stuff on data lines same time as LCD driver)  
    PORTA &= 0;  // disable internal pull-up resistors

    PORTD |= (1 << PD4);        // set R/W (read)
    PORTD &= ~(1 << PD2);       // clear RS signal

    while (busy_flag) {
        ENABLE_HIGH;
        busy_flag = (PINA & (1 << PA7)); 
        ENABLE_LOW;
    }
}

void write_lcd_data(bool data_type, uint8_t data){
    DDRA |= 0xFF;   // output
    PORTD &= ~(1 << PD4);       // clear R/W (write)

    if (data_type == INSTRUCTION){
        PORTD &= ~(1 << PD2);  
    } else if (data_type == DATA){
        PORTD |= (1 << PD2);
    } 

    PORTA &= 0;   // clear data on pins for next write

    PORTA |= data;
    
    ENABLE_HIGH;
    ENABLE_LOW;

}

void init_lcd(){
    DDRD |= (1 << PD2) | ( 1 << PD3) | (1 << PD4);   
    _delay_ms(50); // wait for VCC/POR
    
    // 1st blind function set
    write_lcd_data(INSTRUCTION, 0b00111000); 
    _delay_ms(5);
    
    // 2nd blind function set
    write_lcd_data(INSTRUCTION, 0b00111000); 
    _delay_us(150);
    
    // 3rd blind function set (Top instruction in your image)
    write_lcd_data(INSTRUCTION, 0b00111000); 

    // The LCD's busy flag is now active and reliable. 
    
    // wait_for_low_busy_flag();
    write_lcd_data(INSTRUCTION, 0b00111000); // 4th FS: 2 line display, small font
    
    wait_for_low_busy_flag();
    write_lcd_data(INSTRUCTION, 0b00001000); // Display off 
    
    wait_for_low_busy_flag();
    write_lcd_data(INSTRUCTION, 0b00000001); // Clear display
    
    wait_for_low_busy_flag();
    write_lcd_data(INSTRUCTION, 0b00000110); // Entry mode set
    
    wait_for_low_busy_flag();
    write_lcd_data(INSTRUCTION, 0b00001111); // Display on, cursor on, blink on
    
    wait_for_low_busy_flag();
    write_lcd_data(DATA, 'A');
}

void update_lcd(){

}