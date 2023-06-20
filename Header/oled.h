#ifndef OLED_H
#define OLED_H

#include <avr/pgmspace.h>
#include <avr/power.h>

// LIBARY BY https://github.com/wagiminator/ATtiny13-TinyOLEDdemo/tree/main
// HUGE THANKS

void OLED_init(void);
void OLED_printC(char ch);
void OLED_cursor(uint8_t xpos, uint8_t ypos);
void OLED_clear(void);
#endif