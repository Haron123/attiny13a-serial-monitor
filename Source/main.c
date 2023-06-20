#include <avr/io.h>
#include <avr/power.h>
#include "../Header/oled.h"
#include "../Header/serial_in.h"
#include "../Header/time.h"

int main(void)
{	
	clock_prescale_set(clock_div_1);
	setup_input_pins();
	init_system_tick();

	OLED_init();
	OLED_clear(); 
	OLED_cursor(0,0);

	DDRB |= (1 << PB2);
	PORTB &= ~(1 << PB2);
	
	while(1)
	{	
	}
}