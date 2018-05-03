#include "red_gpio.h"

extern char in_experiment_mode;

void initialize_red_gpio() {

  INIT_RED_GPIO;

  // Set up Data Direction to output for "yellow". Struct define in .h file
  SET_BIT(*red_gpio.ddr, red_gpio.pin);

  //red_gpio_period = 2000;
  red_gpio_period = RED_GPIO_PERIOD_DEFAULT;

  flash_led(&red_gpio, 0);
}

// The "job" executed at each release of the task
int red_gpio_task() {
  TOGGLE_BIT(*(red_gpio.port), red_gpio.pin);
  if (in_experiment_mode) {
    ++red_toggle_count;
  }
  return 1;
}
