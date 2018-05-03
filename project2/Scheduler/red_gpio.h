#ifndef RED_GPIO_H_
#define RED_GPIO_H_

#include <inttypes.h>
#include <util/delay.h>

#include "common.h"

#define RED_GPIO_PERIOD_DEFAULT 100

// Use the IO struct for red_gpio led
// HERE, set to PORT B, Pin 4 (Pin8 on board). If moved, change this only.
#define INIT_RED_GPIO red_gpio = (IO_struct) { &DDRB, &PORTB, 4, &PINB };

IO_struct red_gpio;

// modifiable period of red. default to 100ms (10Hz)
volatile uint64_t red_gpio_period;

// used in experiment to track toggles
volatile uint64_t red_toggle_count;

void initialize_red_gpio();

int red_gpio_task();

#endif
