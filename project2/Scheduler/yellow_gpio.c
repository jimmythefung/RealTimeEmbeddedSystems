#include "yellow_gpio.h"

extern int experiment;
extern char in_experiment_mode;
extern char set_up_experiment;

void initialize_yellow_gpio() {

  INIT_YELLOW_GPIO;
  yellow_gpio_ISR_period = YELLOW_ISR_PERIOD;

  // Set up Data Direction to output for "yellow". Struct define in .h file
  SET_BIT(*yellow_gpio.ddr, yellow_gpio.pin);

  // YELLOW GPIO: timer 3, prescaler 64, period defined in .h file
  SetUpTimerCTC(3, 64, yellow_gpio_ISR_period);

  timer3_ticks = 0;
  //yellow_gpio_period = 2000;
  yellow_gpio_period = YELLOW_GPIO_PERIOD_DEFAULT;

  yellow_toggle_count = 0;

  flash_led(&yellow_gpio, 0);
}

// The "job" executed at each release of the task
void yellow_gpio_task() {
  TOGGLE_BIT(*(yellow_gpio.port), yellow_gpio.pin);
  if (in_experiment_mode) {
    ++yellow_toggle_count;
  }
}

/****************************************************************************
   ISR for YELLOW GPIO : Toggle LED on PORTD6, Pin12
****************************************************************************/
// Timer set up in initialization function above
// Timer set up in defined in timers.c always enables COMPA

ISR(TIMER3_COMPA_vect) {
  if (set_up_experiment) return;
  
  static uint64_t next_release = 0;

  if (timer3_ticks == next_release) {
    // execute the task. It is only 1 line so probably better to make it inline
    // but will keep this way for consistency across all tasks.
    yellow_gpio_task();
    next_release = timer3_ticks + yellow_gpio_period/yellow_gpio_ISR_period;
  }

  // Insert delay if running one of these experiments ...
  // Cannot use variable to set delay -- it requires a constant.
  switch(experiment) {
    case (3): _delay_ms(20); break;
    case (5): _delay_ms(30); break;
    case (7): _delay_ms(105); break;
    case (8): sei(); _delay_ms(105); break;
    default: ;
  }

  // increment here because want task to release at time 0
  timer3_ticks++;
}
