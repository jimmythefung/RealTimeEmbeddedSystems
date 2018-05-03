/*
To find locations (i.e. ports) of I/O, consult the AStar pinout.

The information has been summarized in the AStar DataCheatSheet document
on Github.

Control of the I/O can be found in the AtMega Datasheet.
*/

/*
This is a demonstration of interacting with the on-board leds.
Yellow LED = Port C, pin 7
Green LED = Port D, pin 5
*/

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// char blink = 0;
// volatile long counter = 0;

char yellowReleaseState = 0;
char greenReleaseState = 0;
int green_steps = 0;
int yellow_steps = 0;
long elapsed = 0;

int main() {
	/**
	 * Initialtize Data Direction Register (DDR) for LEDs and buttons
	 */
	DDRC |= (1 << DDC7); // Yellow; PortC; Pin7
	DDRD |= (1 << DDD5); // Green; PortD; Pin5
	DDRB &= ~(1 << DDB3);  // Set Button A (PortB, Pin3) as input
	DDRB &= ~(1 << DDB0);  // Set Button C (PortB, Pin0) as input
	PORTB |= (1 << PORTB3); // Enable Button Pull Up resister
	PORTB |= (1 << PORTB0);


	/**
	* Initialize Port Registers
	*/
	PCICR |= ( 1 << PCIE0 ); // Sets PCIE0, which maps to (and enable) register PCMSK0 which controls interrupts 0 to 7
	PCMSK0 |= ( 1 << PCINT3 ); // Enable interrupt on Button A (PCINT3)
	PCMSK0 |= ( 1 << PCINT0 ); // Enable interrupt on Button C (PCINT0)


	/**
	* Sanity Check - Yellow solid, then yellow off, then green solid.
	* 
	*/
	PORTC |= (1 << PORTC7); // Yellow on
	_delay_ms(1500);
	PORTC ^= (1 << PORTC7); // Yellow off
	PORTD &= ~(1 << PORTD5); // Green solid




	/**
	 * Main Program
	 */
	sei();
	while(1){
		_delay_ms(250);
		green_steps++;
		elapsed = elapsed + 250;

		// Green LED Logic
		if (greenReleaseState==1){
			if (green_steps%2 != 0){ // 250ms, 750ms, 1250ms,... etc
				// Set green led on
				PORTD &= ~(1 << PORTD5);
			}
			else{
				// Set green off
				PORTD |= (1 << PORTD5);
			}

		} else if (greenReleaseState==2){
			// set green off
			PORTD |= (1 << PORTD5);

		} else{
			// Set green led on
			PORTD &= ~(1 << PORTD5);

		}

		// Yellow LED Logic
		if (elapsed%1250==0){ // 1250ms, 3000ms, 3750ms, 5000ms,... etc
			yellow_steps++;
			if (yellowReleaseState==1){
			
				if(yellow_steps%2 != 0){ // Odd steps: 1250ms, 3750ms, ...etc.
					// Set yellow on
					PORTC |= (1 << PORTC7);
				}
				else{
					// Set yellow off
					PORTC &= ~(1 << PORTC7);
				}
			}

		}
		if (yellowReleaseState==2){
			// set yellow off
			PORTC &= ~(1 << PORTC7);
		} else if (yellowReleaseState==3){
			// Set yellow led on
			PORTC |= (1 << PORTC7);
		} else{
			;
		}


		// Reset timer to prevent overflow
		if (elapsed==5000){
			// reset elapse
			elapsed = 0;

			// reset green steps
			if (green_steps%2==0){
				green_steps=0;
			}else{
				green_steps=1;
			}

			// reset yellow steps
			if (yellow_steps%2==0){
				yellow_steps=0;
			}else{
				yellow_steps=1;
			}
		}
	}
	return 0;
}


ISR(PCINT0_vect) {

	// Turn on yellow to indicate inside ISR
	PORTC |= (1 << PORTC7);

	// Check if Button A is pressed
	if(0==(PINB & (1 << PCINT3))){
		_delay_ms(10);
		if(0==(PINB & (1 << PCINT3))){
			if(greenReleaseState==0){ // 0=solid, 1=blink, 2=off
				
				greenReleaseState=1;

			} else if (greenReleaseState==1) {
				
				greenReleaseState=2;
			
			} else {
				
				greenReleaseState=0;
			
			}
		}
	}

	// Check if Button C is pressed
	if(0==(PINB & (1 << PCINT0))){
		_delay_ms(10);
		if(0==(PINB & (1 << PCINT0))){
			if(yellowReleaseState==0){ // 0=solid, 1=blink, 2=off
				
				yellowReleaseState=1;

			} else if (yellowReleaseState==1) {
				
				yellowReleaseState=2;
			
			} else {
				
				yellowReleaseState=0;
			
			}
		}
	}
}


// Reference
//if(0==(PINB & (1 << PCINT3))) // Button A pressed
//if(0==(PINB & (1 << PCINT0))) // Button C pressed