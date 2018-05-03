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
#include <stdbool.h>
#include <stdio.h>
#include <VirtualSerial.h>
#include <stdlib.h>

#define clearBit( port, pin ) port &= ~(1 << pin )
#define setBit( port, pin ) port |= (1 << pin )
#define toggleBit( port, pin ) port ^= (1 << pin )
#define bitMask( bit ) (1 << bit )
#include "motor.c"

volatile char directionToggle = 0;
volatile int dutyCycleLevel = 10;
volatile char encoderPolarity;

typedef struct Node {
	char data;
	struct Node* next;
	struct Node* prev;
} Node;
typedef struct List{
	struct Node* head;
	struct Node* tail;
	int size;
} List;

void enQ(List* L, char data){
	Node* n = malloc(sizeof(Node));
	n->data = data;
	n->next = L->head;
	n->prev = NULL;
	L->head = n;
	L->size++;
	if(n->next==NULL){
		L->tail=n;
	}else{
		(n->next)->prev = n;
	}
}
char deQ(List *L){
	if(L->size > 0){
		char result = (L->tail)->data;

		if(L->size==1){
			L->head = NULL;
			free(L->tail);
			L->tail = NULL;
		}else{
			L->tail = (L->tail)->prev;
			free( (L->tail)->next );
			(L->tail)->next = NULL;
		}
		L->size--;
		return result;
	}
	return NULL;
}

void printBuffer(List *L){
	int i = 0;
	Node* it = L->head;
	printf("Buffer: ");
	while ( i<(L->size) ){
		printf("%c ", it->data);
		it = it->next;
		i++;
	}
	printf("\r\n");
}

void sanityCheck(){
	// Set green LED output for sanity blinker
	DDRD |= (1 << DDD5); // Green;  PortD; Pin5
	int i = 0;
	while (i<3){
		PORTD &= ~(1 << PORTD5); // Green on
		_delay_ms(200);
		PORTD |= (1 << PORTD5); // Green off
		_delay_ms(200);
		i++;
	}
}
void setLEDOutput(){
	// Data Direction Register determines input/output modes
	DDRB |= (1 << DDB0); // Red;    PortB; Pin0
	DDRC |= (1 << DDC7); // Yellow; PortC; Pin7
	DDRD |= (1 << DDD5); // Green;  PortD; Pin5
}


void setButtonInput(){
	// Button A as input
	DDRB &= ~(1 << DDB3);  // Set Button A (PortB, Pin3) as input
	PORTB |= (1 << PORTB3); // Enable Button Pull Up resister A

	// Button B as input
	DDRD &= ~(1 << DDD5);
	PORTD |= (1 << PORTD5);
}

char getEncoderPolarity(){
	global_counts_m2 = 0;
	char polarity = 2; // returning 2 means motor isn't powered on.
	sei();
	setDutyCycle(10);

	// Turn motor on for a quarter revolution to calibrate
	while(abs(global_counts_m2)<1000){
		if (global_counts_m2 > 500){
			polarity = 1;
			break;
		}

		if (global_counts_m2 < -500){
			polarity = 0;
			break;
		}
	}

	OffMotor2();
	cli();
	global_counts_m2 = 0;
	return polarity;
}

void setMotorForward(char encoderPolarity){
	if (encoderPolarity==0){
		motorForward();
	}else{
		motorBackward();
	}
}

void setMotorBackward(char encoderPolarity){
	if (encoderPolarity==0){
		motorBackward();
	}else{
		motorForward();
	}
}

int button_A_is_pressed(){
	int isPressed = 0==(PINB & (1<<PORTB3));
	if (isPressed){
		while(0==(PINB&(1<<PORTB3))){ // still being pressed. wait until release.
			__asm__("nop\n\t"); 
		}
		return 1;
	}
	return 0;
}

int button_B_is_pressed(){
	int isPressed = 0==(PIND&(1<<PORTD5));
	if (isPressed){
		while(0==(PIND&(1<<PORTD5))){ // still being pressed. wait until release.
			__asm__("nop\n\t"); 
		}
		return 1;
	}
	return 0;
}

void processBufferInstructions(List* buffer){
	if (buffer->size > 0){
		char instruction = deQ(buffer);

		switch(instruction){
			case 'A' :
				printf("Processing instruction A - reverse\r\n");
				OffMotor2();
				global_counts_m2 = 0;
				_delay_ms(500);
				if (directionToggle){
					setMotorBackward(encoderPolarity);
				}else{
					setMotorForward(encoderPolarity);
				}
				directionToggle ^= 1;
				setDutyCycle(dutyCycleLevel);
				break;

			case 'B' :
				printf("Processing instruction B - Change speed\r\n");
				OffMotor2();
				_delay_ms(500);
				dutyCycleLevel+=10;
				setDutyCycle(dutyCycleLevel);
				printf("Duty cycle level: %i\r\n", dutyCycleLevel);
				if(dutyCycleLevel>40){
					dutyCycleLevel=0;
					printf("Duty cycle level: %i\r\n", dutyCycleLevel);
				}
				break;

			default :
				printf("Unknown instructions %c\r\n", instruction);
		}
	}
}

int isOnMotor2(){
	return m2_control & (1<<m2_pin);
}

int main() {

	cli();

	// initialization routine for the serial communication
	sanityCheck(); // sanity comes first because the LED setup changes push button PCINT registers used by other codes
	SetupHardware();
	setButtonInput();
	setupMotor2();
	setupEncoder();

	// button B
	DDRD &= ~(1 << DDD5);
	PORTD |= (1 << PORTD5);

	// Power on Port E6 for random testing
	DDRE |= (1 << PORTE6);
    PORTE |= (1 << PORTE6);

	// determine encoder polarity
	encoderPolarity = getEncoderPolarity(); //1=encoder counts up, 0 otherwise

	// initialize buffer to queue up user inputs
	List* buffer = malloc(sizeof(List));
	buffer->head = NULL;
	buffer->tail = NULL;
	buffer->size = 0;

	// Main loop
	sei();
	volatile char c;
	while(1){
		USB_Mainloop_Handler();

		// handle button A input
		if(button_A_is_pressed()){
			enQ(buffer, 'A');
			printf("Button A pressed! ");
			printBuffer(buffer);

		}

		// handle button B input
		if(button_B_is_pressed()){
			enQ(buffer, 'B');
			printf("Button B pressed! ");
			printBuffer(buffer);

		}

		// Stops the motor upon 360 degree
		if(abs(global_counts_m2) >= 2249) {
			global_counts_m2 = 0;
			setDutyCycle(0);
		}

		// Process buffer instructions when motor is ready
		if (!isOnMotor2()){
			processBufferInstructions(buffer);
		}


    }
	return 0;
}