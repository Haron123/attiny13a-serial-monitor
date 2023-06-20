# attiny13a-serial-monitor
a single data wire, serial monitor on an attiny13a using a 128x64 oled ssd1306

HUGE THANKS TO WAGIMINATOR FOR HIS OLED LIBARY https://github.com/wagiminator/ATtiny13-TinyOLEDdemo/tree/main
FOR HIS OLED LIBARY

connect the Host with the PB1 pin on the attiny13a, PB2 is used for looking into the internal timings

COMPILE USING AVR-GCC WITH Oz OPTIMIZATION

# PROTOCOL SPECIFICS :  
Bittimes are 45us  
  
Send one : Set line low for 20us, set it high for atleast 25us  
Send zero : Set line low for 35us, then set it high for atleast 10us  
Send Reset : set line low for 5us, then set it high for atleast 40us  
  
Sending any ASCII with lsb first, starting with a reset, will display the character.  
  
## COMMANDS :  
For a line break send a NULL (0x00) via the bus and wait 150us  
For resetting and clearing the Display send a (0x01) via the bus and wait 200ms (yes milliseconds)  
IF YOU HAVE A COMMAND REQUEST YOU CAN TRY TO CONTACT ME ABOUT IT, I GOT ENOUGH CODESPACE LEFT FOR A FEW MAYBE  
  
After sending a Byte successfully (which the slave wont tell you), you gotta wait another 150us for the    
Slave to write to the display, if you need a higher speed you can also wait less   
(no less than 100us though even at 100us issues arrise), recommended minimum is 120us. and Stable value is 150us  
