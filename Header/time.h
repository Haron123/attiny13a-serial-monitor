#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <avr/io.h>

#define MAX_TIME 255

#define TIMER_CLOCK 9600000
#define PRESCALER 1
#define SECOND (TIMER_CLOCK / PRESCALER)
#define MILLI_SECOND (SECOND / 1000)
#define MICRO_SECOND (MILLI_SECOND / 1000)

// KEEP THIS UPDATED IF YOU CHANGE THE CONFIG : 
// Current settings suggest an overflow occurs every 212 microseconds

// Simplifies the transition to different value ranges and aids in clarity
typedef uint8_t tTime;

/**
 * @brief Initializes the system tick timer.
 * The system tick starts running as soon as this function is executed.
 * It is used to work with time without using an additional timer.
 */
void init_system_tick();

/**
 * @brief Converts a time in microseconds to system ticks.
 * @param us The time to be converted, specified in microseconds.
 * @retval The specified microseconds as system ticks.
 */
tTime us_as_time(uint8_t us);

/**
 * @brief Pauses the system for the specified amount of time.
 * @param us The time to wait, specified in microseconds.
 * @retval None
 */
void delay_us(uint16_t time);

/**
 * @brief Retrieves the current value of the system tick and returns it.
 * @retval The current value of the system tick.
 */
tTime time_now();

/**
 * @brief Compares the current time to a specified time.
 * @retval The time to be compared.
 */
tTime time_passed(tTime since);

#endif
