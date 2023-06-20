// tinyOLEDdemo - controlling an I²C OLED with an ATtiny13
//
// This is just a little demo on how to use an I²C OLED with the limited
// capabilities of an ATtiny13.
//
// The I²C protocol implementation is based on a crude bitbanging method.
// It was specifically designed for the limited resources of ATtiny10 and
// ATtiny13, but should work with some other AVRs as well.
// To make the code as compact as possible, the following restrictions apply:
// - the clock frequency of the MCU must not exceed 4.8 MHz,
// - the slave device must support fast mode 400 kbps (is mostly the case),
// - the slave device must not stretch the clock (this is usually the case),
// - the acknowledge bit sent by the slave device is ignored.
// If these restrictions are observed, the implementation works almost without
// delays. An SCL HIGH must be at least 600ns long in Fast Mode. At a maximum
// clock rate of 4.8 MHz, this is shorter than three clock cycles. An SCL LOW
// must be at least 1300ns long. Since the SDA signal has to be applied anyway,
// a total of at least six clock cycles pass. Ignoring the ACK signal and
// disregarding clock stretching also saves a few bytes of flash. A function
// for reading from the slave was omitted because it is not necessary here.
// Overall, the I2C implementation only takes up 56 bytes of flash.
//
// Ralph Doncaster (nerdralph) pointed out that the SSD1306 can be controlled
// much faster than specified. Therefore an MCU clock rate of 9.6 MHz is also
// possible in this case.
//
// Don't forget the pull-up resistors on the SDA and SCL lines! Many modules,
// such as the SSD1306 OLED module, have already integrated them.
//
// The functions for the OLED are adapted to the SSD1306 128x32 or 128x64 OLED module,
// but they can easily be modified to be used for other modules. In order to
// save resources, only the basic functionalities are implemented.  
//
//    +-----------------------------+
// ---|SDA +--------------------+   |
// ---|SCL |    SSD1306 OLED    |   |
// ---|VCC |   128x32 (or 64)   |   |
// ---|GND +--------------------+   |
//    +-----------------------------+
//
// Controller: ATtiny13
// Core:       MicroCore (https://github.com/MCUdude/MicroCore)
// Clockspeed: 9.6 MHz internal
// BOD:        BOD disabled
// Timing:     Micros disabled
// Leave the rest on default settings. Don't forget to "Burn bootloader"!
// No Arduino core functions or libraries are used. Use the makefile to
// compile without Arduino IDE.
//
// Font used in this demo was adapted from Neven Boyanov and Stephen Denne.
// ( https://github.com/datacute/Tiny4kOLED )
//
// A big thank you to Ralph Doncaster (nerdralph) for his optimization tips.
// ( https://nerdralph.blogspot.com/ , https://github.com/nerdralph )
//
// 2020 by Stefan Wagner 
// Project Files (EasyEDA): https://easyeda.com/wagiminator
// Project Files (Github):  https://github.com/wagiminator
// License: http://creativecommons.org/licenses/by-sa/3.0/

// Select the screen size

//#define SCREEN_128x32
#define SCREEN_128x64

#if defined(SCREEN_128x32) && defined(SCREEN_128x64)
#error "Please define either SCREEN_128x32 or SCREEN_128x64 but not both!"
#endif
#if !defined(SCREEN_128x32) && !defined(SCREEN_128x64)
#error "Please define one of SCREEN_128x32 or SCREEN_128x64!"
#endif

#if defined(SCREEN_128x32)
#define PAGES     4
#define MULTIPLE  1
#else
#define PAGES     8
#define MULTIPLE  2
#endif


#define __DELAY_BACKWARD_COMPATIBLE__ 1       // less delay accuracy saves 16 bytes flash

// Libraries
#include <avr/io.h>
#include <avr/pgmspace.h>

// Pin definitions
#define I2C_SDA         PB4                   // serial data pin
#define I2C_SCL         PB3                   // serial clock pin

// -----------------------------------------------------------------------------
// I2C Master Implementation (Write only)
// -----------------------------------------------------------------------------

// I2C macros
#define I2C_SDA_HIGH()  DDRB &= ~(1<<I2C_SDA) // release SDA   -> pulled HIGH by resistor
#define I2C_SDA_LOW()   DDRB |=  (1<<I2C_SDA) // SDA as output -> pulled LOW  by MCU
#define I2C_SCL_HIGH()  DDRB &= ~(1<<I2C_SCL) // release SCL   -> pulled HIGH by resistor
#define I2C_SCL_LOW()   DDRB |=  (1<<I2C_SCL) // SCL as output -> pulled LOW  by MCU

// I2C init function
static void I2C_init(void) {
  DDRB  &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // pins as input (HIGH-Z) -> lines released
  PORTB &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // should be LOW when as ouput
}

// I2C transmit one data byte to the slave, ignore ACK bit, no clock stretching allowed
static void I2C_write(uint8_t data) {
  for(uint8_t i = 8; i; i--) {            // transmit 8 bits, MSB first
    I2C_SDA_LOW();                        // SDA LOW for now (saves some flash this way)
    if (data & 0x80) I2C_SDA_HIGH();      // SDA HIGH if bit is 1
    I2C_SCL_HIGH();                       // clock HIGH -> slave reads the bit
    data<<=1;                             // shift left data byte, acts also as a delay
    I2C_SCL_LOW();                        // clock LOW again
  }
  I2C_SDA_HIGH();                         // release SDA for ACK bit of slave
  I2C_SCL_HIGH();                         // 9th clock pulse is for the ACK bit
  asm("nop");                             // ACK bit is ignored, just a delay
  I2C_SCL_LOW();                          // clock LOW again
}

// I2C start transmission
static void I2C_start(uint8_t addr) {
  I2C_SDA_LOW();                          // start condition: SDA goes LOW first
  I2C_SCL_LOW();                          // start condition: SCL goes LOW second
  I2C_write(addr);                        // send slave address
}

// I2C stop transmission
static void I2C_stop(void) {
  I2C_SDA_LOW();                          // prepare SDA for LOW to HIGH transition
  I2C_SCL_HIGH();                         // stop condition: SCL goes HIGH first
  I2C_SDA_HIGH();                         // stop condition: SDA goes HIGH second
}

// -----------------------------------------------------------------------------
// OLED Implementation
// -----------------------------------------------------------------------------

// OLED definitions
#define OLED_ADDR       0x78                  // OLED write address
#define OLED_CMD_MODE   0x00                  // set command mode
#define OLED_DAT_MODE   0x40                  // set data mode
#if defined(SCREEN_128x32)
#define OLED_INIT_LEN   12                    // 12: no screen flip, 14: screen flip
#else
#define OLED_INIT_LEN   7                   // 5: no screen flip, 7: screen flip
#endif

// OLED init settings
/*
const uint8_t OLED_INIT_CMD[] PROGMEM = {
  0xA8, 0x3F,       // set multiplex (HEIGHT-1): 31 for 128x32, 63 for 128x64
  0x8D, 0x14,       // enable charge pump
  0xA1, 0xC8,        // flip the screen
  0xAF,             // switch on OLED

};
*/
// Standard ASCII 5x8 font (adapted from Neven Boyanov and Stephen Denne)
const uint8_t OLED_FONT[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, //   0 
  0x00, 0x00, 0x2f, 0x00, 0x00, // ! 1 
  0x00, 0x07, 0x00, 0x07, 0x00, // " 2 
  0x14, 0x7f, 0x14, 0x7f, 0x14, // # 3 
  0x24, 0x2a, 0x7f, 0x2a, 0x12, // $ 4 
  0x62, 0x64, 0x08, 0x13, 0x23, // % 5 
  0x36, 0x49, 0x55, 0x22, 0x50, // & 6 
  0x00, 0x05, 0x03, 0x00, 0x00, // ' 7 
  0x00, 0x1c, 0x22, 0x41, 0x00, // ( 8 
  0x00, 0x41, 0x22, 0x1c, 0x00, // ) 9 
  0x14, 0x08, 0x3E, 0x08, 0x14, // * 10
  0x08, 0x08, 0x3E, 0x08, 0x08, // + 11
  0x00, 0x00, 0xA0, 0x60, 0x00, // , 12
  0x08, 0x08, 0x08, 0x08, 0x08, // - 13
  0x00, 0x60, 0x60, 0x00, 0x00, // . 14
  0x20, 0x10, 0x08, 0x04, 0x02, // / 15
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0 16
  0x00, 0x42, 0x7F, 0x40, 0x00, // 1 17
  0x42, 0x61, 0x51, 0x49, 0x46, // 2 18
  0x21, 0x41, 0x45, 0x4B, 0x31, // 3 19
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4 20
  0x27, 0x45, 0x45, 0x45, 0x39, // 5 21
  0x3C, 0x4A, 0x49, 0x49, 0x30, // 6 22
  0x01, 0x71, 0x09, 0x05, 0x03, // 7 23
  0x36, 0x49, 0x49, 0x49, 0x36, // 8 24
  0x06, 0x49, 0x49, 0x29, 0x1E, // 9 25
  0x00, 0x36, 0x36, 0x00, 0x00, // : 26
  0x00, 0x56, 0x36, 0x00, 0x00, // ; 27
  0x08, 0x14, 0x22, 0x41, 0x00, // < 28
  0x14, 0x14, 0x14, 0x14, 0x14, // = 29
  0x00, 0x41, 0x22, 0x14, 0x08, // > 30
  0x02, 0x01, 0x51, 0x09, 0x06, // ? 31
  0x32, 0x49, 0x59, 0x51, 0x3E, // @ 32
  0x7C, 0x12, 0x11, 0x12, 0x7C, // A 33
  0x7F, 0x49, 0x49, 0x49, 0x36, // B 34
  0x3E, 0x41, 0x41, 0x41, 0x22, // C 35
  0x7F, 0x41, 0x41, 0x22, 0x1C, // D 36
  0x7F, 0x49, 0x49, 0x49, 0x41, // E 37
  0x7F, 0x09, 0x09, 0x09, 0x01, // F 38
  0x3E, 0x41, 0x49, 0x49, 0x7A, // G 39
  0x7F, 0x08, 0x08, 0x08, 0x7F, // H 40
  0x00, 0x41, 0x7F, 0x41, 0x00, // I 41
  0x20, 0x40, 0x41, 0x3F, 0x01, // J 42
  0x7F, 0x08, 0x14, 0x22, 0x41, // K 43
  0x7F, 0x40, 0x40, 0x40, 0x40, // L 44
  0x7F, 0x02, 0x0C, 0x02, 0x7F, // M 45
  0x7F, 0x04, 0x08, 0x10, 0x7F, // N 46
  0x3E, 0x41, 0x41, 0x41, 0x3E, // O 47
  0x7F, 0x09, 0x09, 0x09, 0x06, // P 48
  0x3E, 0x41, 0x51, 0x21, 0x5E, // Q 49
  0x7F, 0x09, 0x19, 0x29, 0x46, // R 50
  0x46, 0x49, 0x49, 0x49, 0x31, // S 51
  0x01, 0x01, 0x7F, 0x01, 0x01, // T 52
  0x3F, 0x40, 0x40, 0x40, 0x3F, // U 53
  0x1F, 0x20, 0x40, 0x20, 0x1F, // V 54
  0x3F, 0x40, 0x38, 0x40, 0x3F, // W 55
  0x63, 0x14, 0x08, 0x14, 0x63, // X 56
  0x07, 0x08, 0x70, 0x08, 0x07, // Y 57
  0x61, 0x51, 0x49, 0x45, 0x43, // Z 58
  0x00, 0x7F, 0x41, 0x41, 0x00, // [ 59
  0x02, 0x04, 0x08, 0x10, 0x20, // \ 60
  0x00, 0x41, 0x41, 0x7F, 0x00, // ] 61
  0x04, 0x02, 0x01, 0x02, 0x04, // ^ 62
  0x40, 0x40, 0x40, 0x40, 0x40  // _ 63
};

// OLED init function
void OLED_init(void) {
  I2C_init();                             // initialize I2C first
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(0xA8);
  I2C_write(0x3F);
  I2C_write(0x8D);
  I2C_write(0x14);
  I2C_write(0xA1);
  I2C_write(0xC8);
  I2C_write(0xAF);
  I2C_stop();                             // stop transmission
}

// OLED print a character
void OLED_printC(char ch) {
  uint16_t offset = ch - 32;              // calculate position of character in font array
  offset += offset << 2;                  // -> offset = (ch - 32) * 5
  I2C_start(OLED_ADDR);
  I2C_write(OLED_DAT_MODE);
  I2C_write(0x00);                        // print spacing between characters
  for(uint8_t i=5; i; i--) I2C_write(pgm_read_byte(&OLED_FONT[offset++])); // print character
  I2C_stop();
}

// OLED set the cursor
void OLED_cursor(uint8_t xpos, uint8_t ypos) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(0xB0 | (ypos & 0x07));        // set start page
  I2C_write(xpos & 0x0F);                 // set low nibble of start column
  I2C_write(0x10 | (xpos >> 4));          // set high nibble of start column
  I2C_stop();                             // stop transmission
}

// OLED clear screen
void OLED_clear(void) {
  for (uint8_t i = 0; i < PAGES; i++) {   // clear screen line by line
    OLED_cursor(0, i);
    I2C_start(OLED_ADDR);                   // start transmission to OLED
    I2C_write(OLED_DAT_MODE);               // set data mode
    for(uint8_t i=128; i; i--) I2C_write(0x00);
    I2C_stop();                             // stop transmission
  }
}