#ifndef TIMERS_H_
#define TIMERS_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "common.h"

/* Set up a timer using ctc mode.
 * parameter [in] timer_num : 0 (8-bit) or 1,3 (16-bit)
 * parameter [in] prescaler : 1,8,64, ... as indicated in datasheet options
 * parameter [in] ms_period : desired period of ISR
 * parameter [out] : -1 for error in setting up. 1 otherwise.
 *
 * @TODO: pass in a function pointer and use a #define to define the ISR
 *
 * WARNING: You must define the corresponding ISR or the board will reset
 */
 int SetUpTimerCTC( int timer_num, int prescaler, int ms_period );

int SetUpTimer_0(char CSbits, uint8_t match);
int SetUpTimer_1(char CSbits, uint16_t match);
int SetUpTimer_3(char CSbits, uint16_t match);

/* Return clock-select bits based on prescaler option as defined in datasheet */
char GetCSBits(int prescaler);

#endif
