
#define F_CPU 16000000ul	// required for _delay_ms()
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <math.h>

// Uncomment this line if you want to use testload.h and not run houghTransform
// #define __TESTONLY 1

// if not testing, then image file #included in hough.h
#ifndef __TESTONLY
#include "hough.h"
#else
#include "testload.h"
#endif

void setupUART(void) {
	// BAUD set to 57600  <<<<<<
	UBRR1 = ((F_CPU/(16*57600)) - 1);
	UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);		// 8 bit char size
	UCSR1B |= (1 << TXEN1);		// enable transmit
}

void sendChar(uint8_t c) {
	while((UCSR1A & (1<<UDRE1)) == 0);	// wait while data register is NOT empty
	UDR1 = c;
}

void sendString(uint8_t *s) {
	while(*s != 0x00) {
		sendChar(*s);
		s++;
	}
}

// int main(void) {

// 	uint16_t row, col;
// 	uint16_t nextByte;

// 	DDRC |= (1<<PORTC7);
// 	setupUART();

// 	#ifndef __TESTONLY
// 	volatile char dummyVar;

// 	DDRD |= (1 << DDD6);
// 	for (;;) {
// 	PORTD &= ~(1 << PORTD6);
// 	// compiler wants to optimize this all away. volatile dummyVar is an attempt around this
// 	dummyVar = houghTransform( (uint16_t) &red, (uint16_t) &green, (uint16_t) &blue );
// 	PORTD |= (1 << PORTD6);
// 	_delay_ms(2);
// 	}
// 	//sendChar(dummyVar);
// 	//_delay_ms(10);
// 	#endif

// 	for(;;) {

// 		// Use this to test file. ASCII codes are non-alphanumeric in imageXX.h, so
// 		// better to do this with the testload.h rather than imageXX.h
// 		for (row = 0; row < __Height; row++) {
// 			sendChar('R');
// 			for (col = 0; col < __Width; col++) {
// 				nextByte = ((uint16_t) &red) + row*__Width + col;
// 				sendChar(pgm_read_byte_near(nextByte));
// 				_delay_ms(10);
// 			}
// 		}

// 		PORTC |= (1<<PORTC7);
// 		_delay_ms(200);

// 		PORTC &= ~(1<<PORTC7);
// 		_delay_ms(200);
// 	}
// 	return 0;
// }
