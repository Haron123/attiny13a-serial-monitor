#include <avr/io.h>

#define SERIAL_PIN PD2

void setup()
{
  DDRD |= (1 << PD2);
  PORTD |= (1 << PD2);

  char hello_world[13] = "HELLO WORLD!";

  clearDisplay();
  printString(hello_world, sizeof(hello_world));
  lineBreak();
  printString(hello_world, sizeof(hello_world));
}

void loop()
{
  // the '\0' string terminater will be interpreted as linebreak, if you dont want that then keep that in mind
}

// Send the size including the terminator
void printString(char* string, uint8_t string_size)
{
  for(uint8_t i = 0; i < string_size-1; i++)
  {
    printChar(string[i]);
  }
}

void printChar(char c)
{
  sendReset();
  sendAscii(c);
}

void sendAscii(char data)
{
  sendReset();
  for (uint8_t i = 0; i < 8; i++)
  {
    if (data & (1 << i))
    {
      sendOne();
    }
    else
    {
      sendZero();
    }
  }
  delayMicroseconds(150);
}

void clearDisplay()
{
  sendReset();
  sendAscii(0x01);
  delay(200); // Resetting the display means overwriting it, which is very time intense
}

void lineBreak()
{
  sendReset();
  sendAscii(0x00);
  delayMicroseconds(150);
}

void sendZero()
{
  PORTD &= ~(1 << PD2);
  delayMicroseconds(35);
  PORTD |= (1 << PD2);
  delayMicroseconds(10);
}

void sendOne()
{
  PORTD &= ~(1 << PD2);
  delayMicroseconds(20);
  PORTD |= (1 << PD2);
  delayMicroseconds(25);
}

void sendReset()
{
  PORTD &= ~(1 << PD2);
  delayMicroseconds(5);
  PORTD |= (1 << PD2);
  delayMicroseconds(40);
}
