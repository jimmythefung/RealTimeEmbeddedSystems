/*
To find locations (i.e. ports) of I/O, consult the AStar pinout.

The information has been summarized in the AStar DataCheatSheet document
on Github.

Control of the I/O can be found in the AtMega Datasheet.
*/

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include<stdbool.h>
#include <stdio.h>
#include <VirtualSerial.h>
#include "task.c"

typedef struct{
	int (*funptr)();
	int period;
	uint32_t expireTime; // next release time 
	int missedDeadlines;
	int ID;
	int priority;
	int state;		//0=ready, 1=blocking, 2=executing
	int jobsInQueue;
	int qMaxSeen;
} Task;
Task t1, t2, t3, t4, t5, t6, t7, t8; 
Task *tArrSorted[8]  = {&t1, &t3, &t6, &t4, &t2, &t7, &t5, &t8}; // array of pointers to task


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

void setUpATask(
	Task *t,
	int (*f)(),
	int period,
	uint32_t expireTime,
	int missedDeadlines,
	int ID,
	int priority,
	int state,
	int jobsInQueue,
	int qMaxSeen){

	t->funptr = f;
	t->period = period;
	t->expireTime = expireTime;
	t->missedDeadlines = missedDeadlines;
	t->ID = ID;
	t->priority = priority;
	t->state = state;
	t->jobsInQueue = jobsInQueue;
	t->qMaxSeen = qMaxSeen;
}

void initAllTasks(){
	//         task, function,    period, expT, missedDL, ID, priority, state, jobsInQ, qMaxSeen
	setUpATask(&t1,  Invert,      10,     0,    0,        1,  10,       1,     0,       0);
	setUpATask(&t2,  TaskDelay1,  25,     0,    0,        2,  25,       1,     0,       0);
	setUpATask(&t3,  MaxMin,      17,     0,    0,        3,  17,       1,     0,       0);
	setUpATask(&t4,  DelayDelay,  22,     0,    0,        4,  22,       1,     0,       0);
	setUpATask(&t5,  TaskDelay2,  27,     0,    0,        5,  27,       1,     0,       0);
	setUpATask(&t6,  Average,     20,     0,    0,        6,  20,       1,     0,       0);
	setUpATask(&t7,  TaskDelay3,  25,     0,    0,        7,  25,       1,     0,       0);
	setUpATask(&t8,  Hough,       40,     0,    0,        8,  40,       1,     0,       0);
}

void printInfoOfTask(Task *t){
    printf("ID: %i\r\n", t->ID);
    printf("period: %i\r\n", t->period);
    printf("expireTime: %i\r\n", t->expireTime);
    printf("missedDeadlines: %i\r\n", t->missedDeadlines);
    printf("priority: %i\r\n", t->priority);
    printf("state: %i\r\n", t->state);
    printf("jobsInQueue: %i\r\n", t->jobsInQueue);
    printf("qMaxSeen: %i\r\n", t->qMaxSeen);
}

// void printTaskTable(){
// 	volatile int i;
// 	Task *t;
// 	printf("ID  period  expireTime  missedDeadlines      priority  state  jobsInQueue  qMaxSeen\r\n");
	
// 	for(i=0; i<8; i++){
// 		t = tArrSorted[i];
// 		printf("%i   %i      %i          %i                %i        %i      %i            %i\r\n", t->ID, t->period, t->expireTime, t->missedDeadlines, t->priority, t->state, t->jobsInQueue, t->qMaxSeen);
// 	}

// }

void printResult(){
	volatile int i;
	Task *t;
	printf("ID   missedDeadlines  jobsInQueue  qMaxSeen\r\n");
	
	for(i=0; i<8; i++){
		t = tArrSorted[i];
		printf("%i     %i           %i          %i \r\n", t->ID, t->missedDeadlines, t->jobsInQueue, t->qMaxSeen);
	}

}

Task * getHighestPriorityReadyTask(){
	volatile int i;
	Task *t;
	for(i=0; i<8; i++){
		t = tArrSorted[i]; // already sorted by priority descending
		if(t->state == 0){
			return t;
		}
	}
	return NULL;
} 
void releaseTask(Task *t){ int x = t->funptr(); }
void setReady(Task *t){ t->state = 0; }
void setBlocking(Task *t){ t->state = 1; }
void setExecuting(Task *t){ t->state = 2; }
void decrementIfQueued(Task *t){
	if(t->jobsInQueue > 0){ 
		t->jobsInQueue--; 
	} 
}

int main() {	
	// Initialize I/O
	setLEDOutput();   // Set Yellow and Green LED as output
	setButtonInput(); // Set buttons A and B as interrupt
	sanityCheck(); // Blink the LEDs
	timer1_init(); // 1 kHz timer 
	SetupHardware(); // This sets up the USB hardware and stdio

	// Main loop
	initAllTasks();
	sei();
	char c;
	Task *t;
	while(1){
		if(time_ms>60000){
	    	// Handles USB communication
	    	USB_Mainloop_Handler();
	    	if(time_ms%250==0){
	    		PORTD ^= (1 << PORTD5);
	    		printResult();
	    	}
		}


	    // get high priority task that is ready
	    t = getHighestPriorityReadyTask();
	    // if so release it and decrement queue, set state to executing
	    if(t){
	    	setExecuting(t);
	    	decrementIfQueued(t);
	    	releaseTask(t);
	    	setBlocking(t); //A task that has completed its current job and is waiting for the next release should be in the state "blocked."
	    }
	    

	    // LUFA temrinal screen
	    if ((c=fgetc(stdin)) != EOF){
	        //printf("Hello World! You hit %c!\r\n", c);
	        printResult(t);
	        //printTaskTable();
	    }

	    // // blink green for liveness
	    // PORTD ^= (1 << PORTD5);
	    // _delay_ms(250);
		
	}
	return 0;
}


bool taskIsReadyForRelease(Task *t){ return time_ms >= (t->expireTime); }
void setTimeOfNextRelease(Task *t){ t->expireTime += t->period; }
bool stateIsReady(Task *t){ return (t->state==0); }
bool stateIsExecuting(Task *t){ return (t->state==2); }
void incrementMissedDeadline(Task *t){ t->missedDeadlines++; }
void incrementQueue(Task *t){ t->jobsInQueue++; }
void updateMaxQueue(Task *t){
	if (t->jobsInQueue > t->qMaxSeen){
		t->qMaxSeen = t->jobsInQueue;
	}
}
// This is the interrupt service routine for timer1 
ISR(TIMER1_COMPA_vect){
	volatile int i;
	Task *t;

	// Increment ms
	time_ms++;

	// For each task, check if any task is ready to be released
	for(i=0; i<8; i++){
		t = tArrSorted[i];
		// if task is ready to be released
		if(taskIsReadyForRelease(t)){
			setTimeOfNextRelease(t);
			if(stateIsReady(t) || stateIsExecuting(t)){
				// Task misses a deadline. Increment deadline. Increcrement queue.
				incrementMissedDeadline(t);
				incrementQueue(t);
				updateMaxQueue(t);
			}else{ // state is blocked
				// otherwise set it to ready state
				setReady(t);
			}
		}
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
