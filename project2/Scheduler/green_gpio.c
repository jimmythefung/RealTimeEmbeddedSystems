#include "green_gpio.h"

extern int experiment;
extern char in_experiment_mode;
extern char set_up_experiment;

void initialize_green_gpio() {

  INIT_GREEN_GPIO;
  // at 256 prescaler, can't go higher than 1000
  // but at 1024, period of 100 doesn't go evenly.
  // @TODO: reset clock with prescaler 1024 if user wants period > 1000
  //green_gpio_period = 1000;
  green_gpio_period = GREEN_GPIO_PERIOD_DEFAULT;

  // Set up Data Direction to output for "yellow". Struct define in .h file
  SET_BIT(*green_gpio.ddr, green_gpio.pin);

  // GREEN GPIO: timer 1, prescaler 256, period defined in .h file
  // Since we are toggling the LED, we can use the CTC mode
  // However, the COM bits need to be set so that the OC pin toggles
  // on every match. The ISR is used to count the toggles.
  SetUpTimerCTC(1, 256, green_gpio_period);
  SET_BIT(TCCR1A, COM1B0);

  green_toggle_count = 0;

  flash_led(&green_gpio, 0);
}

// The "job" executed at each release of the task
void green_gpio_task() {
  //PORTB ^= (1<<PORTB6);
  if (in_experiment_mode) {
    ++green_toggle_count;
  }
}

/****************************************************************************
   ISR for GREEN GPIO - OC used to toggle. This counts toggles.
****************************************************************************/
// Timer set up in initialization function above
// Timer set up in defined in timers.c always enables COMPA
ISR(TIMER1_COMPA_vect) {
  if (set_up_experiment) return;

  // execute the task. It is only 1 line so probably better to make it inline
  // but will keep this way for consistency across all tasks.
  green_gpio_task();

  // Insert delay if running one of these experiments ...
  // Cannot use variable to set delay -- it requires a constant.
  switch(experiment) {
    case (2): _delay_ms(20); break;
    case (4): _delay_ms(30); break;
    case (6): _delay_ms(105); break;
    default: ;
  }
}
