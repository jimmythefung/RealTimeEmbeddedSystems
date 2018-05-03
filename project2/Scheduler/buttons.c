#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "buttons.h"

void EmptyFunction() {}

void initialize_buttons() {

  INIT_BUTTONA;
  INIT_BUTTONC;

  fn_release_A = EmptyFunction;
  fn_press_A = EmptyFunction;
  fn_release_C = EmptyFunction;
  fn_press_C = EmptyFunction;
}

/* Set up any of th buttons on the board.
* parameter [in] IO_struct button : for A or C as defined above
* parameter [in] release : 1 (true) = call function on release
* parameter [in] callback : function to be called when event happens
* parameter [out] : -1 for error in setting up. 1 otherwise.
*/

int SetUpButton(IO_struct * button, int release, void(*callback)(void)) {

  // Configure the data direction to input
  CLEAR_BIT(*button->ddr, button->pin);
  // Enable Button pull-up resistor
  SET_BIT(*button->port, button->pin);

  // PCICR: Pin Change Interrupt Control Registe
  // PCIE0: Pin Change Interrupt Enable Bit:
  //    Any change on any enabled PCINT7..0 can fire ISR.
  PCICR |= (1 << PCIE0);

  // PCMSK0: Pin Change Mask for Interrupt0, which is for all pins 0 through 7
  // Enable interrupts on Button A (PCINT3) and Button C (PCINT0)
  if (button->pin == BUTTONA) {
    PCMSK0 |= (1 << PCINT3);
  }
  if (button->pin == BUTTONC) {
    PCMSK0 |= (1 << PCINT0);
  }

  // Set up for the callback function to be used when button event happens
  if (button->pin == BUTTONA) {
    if (release) {
      fn_release_A = callback;
    } else {
      fn_press_A = callback;
    }
  }
  if (button->pin == BUTTONC) {
    if (release) {
      fn_release_C = callback;
    } else {
      fn_press_C = callback;
    }
  }
  pinb_previous = PINB & BUTTON_MASK;
  return 0;
}

ISR(PCINT0_vect) {

  uint8_t pinb_now = (PINB & BUTTON_MASK);

  // debounce
  _delay_ms(1);
  if (pinb_now ^ (PINB & BUTTON_MASK)) {
    PORTC |= (1<<PORTC7);
    _delay_ms(100);
    PORTC &= ~(1<<PORTC7);
    return;
  }

  uint8_t pinb_change = (pinb_now ^ pinb_previous);

  // Determine if Button A has changed since the last time
  if (pinb_change & (1 << BUTTONA)) {
    // did it press (0) or release (1) ?
    if (0 == (pinb_now & (1<<BUTTONA))) {
      fn_press_A();
    }
    else {
      fn_release_A();
    }
  }
  // Determine if Button C has changed since the last time
  if (pinb_change & (1 << BUTTONC)) {
    // did it press (0) or release (1) ?
    if (0 == (pinb_now & (1<<BUTTONC))) {
      fn_press_C();
    }
    else {
      fn_release_C();
    }
  }
  pinb_previous = pinb_now;
}
