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
#include <inttypes.h>

// char blink = 0;
// volatile long counter = 0;
uint32_t time_ms = 0;
char yellowOn = 0;
volatile long counter = 0;


void timer1_init(){
	// Setup timer1 channel B using pre-scaler 64.
	TCCR1B |= (1<<CS10);
	TCCR1B |= (1<<CS11);
	TCCR1B |= (1<<WGM12); // Using CTC mode with OCR1A for TOP. This is mode 4.; WGM(n)2 where n is timer

	// Software Clock Interrurpt frequency: 1000 = f_IO / (prescaler*OCR1A)
	OCR1A = 250; // When TCNT==OCR1A, sets TIFR (Timer Interrupt Flag Register), trigger TIMER1_COMPA_vect

	// Enable output compare match interrupt on timer 1 channel A
	TIMSK1 |= (1 << OCIE1A);
}




void sanityCheck(){
	int i = 0;
	while (i<3){
		PORTD &= ~(1 << PORTD5); // Green on
		_delay_ms(200);
		PORTD |= (1 << PORTD5); // Green off
		_delay_ms(200);
		i++;
	}
}


// >>>>> Good to have these reusable helper functions
// Try modifying these to take in buttons or registers to make it
// generic for user e.g. setButtonInput(ButtonA) and set pull-up too

void setButtonInput(){
	// Pin Change Interrupt Control Register
	PCICR |= ( 1 << PCIE0 ); // Sets PCIE0 enable allows pin cahnge interrupt,

	// Note: // Pin Change Mask Register 0 - PCMSK0; Enable/Disable pin change interupt 0 to 7
	// Button A
	DDRB &= ~(1 << DDB3);  // Set Button A (PortB, Pin3) as input
	PORTB |= (1 << PORTB3); // Enable Button Pull Up resister A
	PCMSK0 |= ( 1 << PCINT3 ); // Enable interrupt on Button A (PCINT3)

	// // Button C
	// DDRB &= ~(1 << DDB0);  // Set Button C (PortB, Pin0) as input
	// PORTB |= (1 << PORTB0); // Enable Button Pull Up resister C
	// PCMSK0 |= ( 1 << PCINT0 ); // Enable interrupt on Button C (PCINT0)
}




void setLEDOutput(){
	// Data Direction Register determines input/output modes
	DDRB |= (1 << DDB0); // Red;    PortB; Pin0
	DDRC |= (1 << DDC7); // Yellow; PortC; Pin7
	DDRD |= (1 << DDD5); // Green;  PortD; Pin5

}






int main() {
	// Initialize I/O
	setLEDOutput();   // Set Yellow and Green LED as output
	setButtonInput(); // Set buttons A and B as interrupt


	// Blink the LEDs
	sanityCheck();


	// 1 kHz timer
	timer1_init();


	// Main loop
	sei();


// >>>> THIS should be monitoring the ms_ticks to determine when to go.
// if (ms_ticks > release_time)
// This is perfectly viable way to pseudo-schedule, but different from
// assigment requirement.
	while(1){
		if (yellowOn==1){
			PORTC |= (1 << PORTC7);
		}else{
			PORTC &= ~(1 << PORTC7);
		}

	}
	return 0;
}


// This is the interrupt service routine for timer1
ISR(TIMER1_COMPA_vect){
	// Increment ms
	time_ms++;

	// blink green every 1 sec
	if (time_ms%1000 == 0){
		// toggle yellow
		yellowOn ^= 1;

		/* >>>> Logically correct but I don't advise doing this
		typically. More tasks might want to use the timer and need it to
		keep incrementing.
		*/
		// reset ms variable every 1 seconds
		time_ms = 0;
	}
}


// Button A interrupt
ISR(PCINT0_vect) {
	// Turn on red to indicate inside ISR
	PORTB ^= (1 << PORTB0);

	// Do something when button is release
	//volatile char APressed = (0==( PINB & (1 << PCINT3) )); // Because of pull out resistor, unpressed button makes PINB hi. And pressed makes PINB low.
	if(! (0==(PINB&(1 << PCINT3))) ){ // pressed==0, released==1
		_delay_ms(20);
		if(! (0==(PINB&(1 << PCINT3))) ){
			// Blink green LED at 4Hz
			while(1){
				// Toggle green LED
				PORTD ^= (1 << PORTD5);

				// 125ms delay using while loop
				// >>>> Did ths work or did it get optimized out.
				while (counter < 100000){
					counter++;
				}
				counter = 0;
			}
		}
	}
}


// Reference
//if(0==(PINB & (1 << PCINT3))) // Button A pressed
//if(0==(PINB & (1 << PCINT0))) // Button C pressed
