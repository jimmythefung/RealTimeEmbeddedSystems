#ifndef BUTTONS_H_
#define BUTTONS_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "common.h"

#define BUTTONA 3
#define BUTTONC 0

// Setup is for A and C only because button B does not have corresponding
// PCINT for easy interrupt programming.
//
// WARNING: ButtonC and RED LED share the pin. Do not use both.

#define INIT_BUTTONA _button_A = (IO_struct) { &DDRB, &PORTB, BUTTONA, &PINB }; \
  DDRC |= (1<<DDC7);
#define INIT_BUTTONC _button_C = (IO_struct) { &DDRB, &PORTB, BUTTONC, &PINB }; \
  DDRB |= (1<<DDB0);

IO_struct _button_A;
IO_struct _button_C;

#define BUTTON_MASK ((1<<BUTTONA) | (1<<BUTTONC))

void initialize_buttons();

/* Set up any of th buttons on the board.
 * parameter [in] IO_struct button : for A or C as defined above
 * parameter [in] release : 1 (true) = call function on release
 * parameter [in] callback : function to be called when event happens
 * parameter [out] : -1 for error in setting up. 1 otherwise.
 */
int SetUpButton(IO_struct * button, int release, void(*callback)(void));

void EmptyFunction();

void (*fn_release_A)(void);
void (*fn_press_A)(void);
void (*fn_release_C)(void);
void (*fn_press_C)(void);

uint8_t pinb_previous;

#endif
