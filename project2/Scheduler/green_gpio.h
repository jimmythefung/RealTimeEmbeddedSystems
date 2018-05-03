#ifndef GREEN_GPIO_H_
#define GREEN_GPIO_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "common.h"
#include "timers.h"

#define GREEN_GPIO_PERIOD_DEFAULT 100

// Use the IO struct for green_gpio led
// HERE, set to PORT B, Pin 6 (Pin10/OC1B on board).
#define INIT_GREEN_GPIO green_gpio = (IO_struct) { &DDRB, &PORTB, 6, &PINB };

IO_struct green_gpio;

// an attempt to count green toggles
volatile uint64_t green_toggle_count;

// modifiable period of yellow. default to 100ms (10Hz)
volatile uint64_t green_gpio_period;

void initialize_green_gpio();

void green_gpio_task();

#endif
