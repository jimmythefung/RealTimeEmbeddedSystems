#ifndef YELLOW_GPIO_H_
#define YELLOW_GPIO_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "common.h"
#include "timers.h"

#define YELLOW_ISR_PERIOD 25
#define YELLOW_GPIO_PERIOD_DEFAULT 100

// Use the IO struct for yellow_gpio led
// HERE, set to PORT D, Pin 6 (Pin12 on board). If moved, change this only.
#define INIT_YELLOW_GPIO yellow_gpio = (IO_struct) { &DDRD, &PORTD, 6, &PIND };

IO_struct yellow_gpio;

// used in TIMER3_COMPA ISR
volatile uint64_t timer3_ticks;

// fixed period of yellow gpio ISR. 25ms (40Hz)
volatile uint64_t yellow_gpio_ISR_period;

// modifiable period of yellow. default to 100ms (10Hz)
volatile uint64_t yellow_gpio_period;

// used in experiment to track toggles
volatile uint64_t yellow_toggle_count;

void initialize_yellow_gpio();

void yellow_gpio_task();

#endif
