#include <avr/io.h>
#include <avr/interrupt.h>
#include "../Header/oled.h"
#include "../Header/time.h"

#ifndef SERIAL_IN_H
#define SERIAL_IN_H

#define SERIAL_PIN PB1

#define PINPORT PINB
#define DDRPORT DDRB
#define SERIALPORT PORTB

#define GET_WIRE_STATE (PINPORT & (1 << SERIAL_PIN))

/**
 * @brief sets up the interrupt for the CLOCKPIN
 * on the attiny13a only PB1 has an external pin interrupt
 * @param none
 * @retval none
 */
void setup_input_pins();

#endif

/* PROTOCOL SPECIFICS :
Bittimes are 45us

Send one : Set line low for 20us, set it high for atleast 25us
Send zero : Set line low for 35us, then set it high for atleast 10us
Send Reset : set line low for 5us, then set it high for atleast 40us

Sending any ASCII with lsb first, starting with a reset, will display the character.

COMMANDS :
For a line break send a NULL (0x00) via the bus and wait 150us
For resetting and clearing the Display send a (0x01) via the bus and wait 200ms (yes milliseconds)
IF YOU HAVE A COMMAND REQUEST YOU CAN TRY TO CONTACT ME ABOUT IT, I GOT ENOUGH CODESPACE LEFT FOR A FEW MAYBE

After sending a Byte successfully (which the slave wont tell you), you gotta wait another 150us for the
Slave to write to the display, if you need a higher speed you can also wait less 
(no less than 100us though even at 100us issues arrise), recommended minimum is 120us. and Stable value is 150us
*/