/*
 * basic_motor.c
 *
 * Created: 02/2017
 * Author : Chris Owens and Amy Larson
 */
 #ifdef VIRTUAL_SERIAL
 #include <VirtualSerial.h>
 #else
 #warning VirtualSerial not defined, USB IO may not work
 #define SetupHardware();
 #define USB_Mainloop_Handler();
 #endif


#define F_CPU 16000000ul	// required for _delay_ms()
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>


#define clearBit( port, pin ) port &= ~(1 << pin )
#define setBit( port, pin ) port |= (1 << pin )
#define toggleBit( port, pin ) port ^= (1 << pin )
#define bitMask( bit ) (1 << bit )

// It would be better to use a header, then link this file in, but Makefile is more complicated.
#include "motor.c"

volatile uint32_t ms_tick = 0;

void setupMStimer(void) {
	TCCR3A = 0; // clear for good measure
	TCCR3B = 0x08;	// set CTC mode (clear-timer-on-compare)

	// apparently this writes to the HIGH and LOW capture regs
	OCR3A = 15999;  // 1ms period

	TIMSK3 = 0x02;  // OCIE1A compare match A interrupt Enable
	TCCR3B |= 0x01; // start timer with no prescaler
}

int main(void)
{
	uint8_t state = 0;

	USBCON = 0;

	cli();

	// initialization routine for the serial communication
	SetupHardware();

	setupMotor2();
	setupEncoder();

	/* blink RED LED to confirm board booted up */
	DDRB |= (1 << DDB0);
	PORTB &= ~(1 << PORTB0);
	_delay_ms(500);
	PORTB |= (1 << PORTB0);
	//sendString("Test MSG, to confirm connection!\n");
	_delay_ms(500);
	PORTB &= ~(1 << PORTB0);
	
	// // Encoder power on at Port E6
	DDRE |= (1 << PORTE6);
    PORTE |= (1 << PORTE6);
	
	setDutyCycle(10);	// start motor with 10% duty cycle
	// Set motor to output to turn it on. ()

	OnMotor2();
	sei();

	char fEchoedState = 0;
	while(1)
	{
		switch(state){
			case 0:	// positive direction
				motorForward();
				if (!fEchoedState) {
					printf("State 0\n\r");
					USB_Mainloop_Handler();
					fEchoedState = 1;
				}
				if(global_counts_m2 >= 2249) {
					setDutyCycle(0);
					state = 1;
					fEchoedState = 0;
				}
				break;

			case 1:	// delay 1second
				printf("State 1\n\r");
				USB_Mainloop_Handler();
				_delay_ms(1000);
				state = 2;
				break;

			case 2:	// negative direction
				motorBackward();
				setDutyCycle(10);
				if (!fEchoedState) {
					printf("State 2\n\r");
					USB_Mainloop_Handler();
					fEchoedState = 2;
				}
				if(global_counts_m2 < 0) {
					setDutyCycle(0);
					state = 3;
				}
				break;

			case 3:	// delay 1 second
				printf("State 3\n\r");
				USB_Mainloop_Handler();
				_delay_ms(1000);
				setDutyCycle(10);
				state = 0;
				break;

			default:
				printf("oops\n\r");
				state = 0;
		}
    }
}
