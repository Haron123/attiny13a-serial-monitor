#include "../Header/serial_in.h"

// DUE TO THE TIMER BEING INACCURATE U MIGHT SEE DIFFERENT NUMBERS THAN WHATD YOU EXPECT
// FOR DELAYS, ITS HAND TUNED

// This code isnt well structured i know but i had to save space and speed so its all just puked down here
// im happy if anyone can make this look pretty :D
ISR(INT0_vect)
{	
	uint8_t now = time_now();
	static uint8_t bit_count = 0;
	static uint8_t result = 0;

	static uint8_t x = 0;
	static uint8_t y = 0;
	PORTB |= (1 << PB2);
	while(time_passed(now) < 12); // 10us, send reset until here
	PORTB &= ~(1 << PB2);
	now = time_now();

	if(GET_WIRE_STATE)
	{
		bit_count = 0;
		result = 0;
		return;
	}
	PORTB |= (1 << PB2);
	while(time_passed(now) < 12); // 10us, send the bit until here if no reset occured
	// Get the bit thats on the line
	PORTB &= ~(1 << PB2);
	if(GET_WIRE_STATE)
	{
		result |= (1 << bit_count);
	}

	bit_count++;

	if(bit_count == 8)
	{
		// 8 Bits have been received
		if(x == 21)
		{
			// End of Line
			y = (y+1) & 7;
			x = 0;
			OLED_cursor(0,y);
		}
		if(result)
		{
			if(result == 0x01)
			{
				//Reset display command
				OLED_clear(); 
				OLED_cursor(0,0);
				x = 0;
				y = 0;
				return;
			}
			// Print the Byte
			OLED_printC(result);
			x++;
			return;
		}
		// Next Line command
		y = (y+1) & 7;
		x = 0;
		OLED_cursor(0,y);
		
		// Start anew
		bit_count = 0;
		result = 0;
	}
}

void setup_input_pins()
{
	cli(); // Turn off Global Interrupts	
	
	DDRPORT &= ~(1 << SERIAL_PIN); // Setup Data Direction Register
	
	/*
	https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny13A-Data-Sheet-DS40002307A.pdf
	Chapter 9.3

	ISC01	ISC00	Description
	0		0		The low level of INT0 generates an interrupt request.
	0		1		Any logical change on INT0 generates an interrupt request.
	1		0		The falling edge of INT0 generates an interrupt request.
	1		1		The rising edge of INT0 generates an interrupt request.
	*/
	MCUCR = (1 << ISC01) | (0 << ISC00);

	GIMSK = (1 << INT0) | (0 << PCIE); // Enable External Interrupt INT0
	PCMSK = (1 << SERIAL_PIN); // Use SERIAL_PIN as external interrupt

	sei(); // Turn on Global interrupts
}