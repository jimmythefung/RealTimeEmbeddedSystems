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
#define LENGTH(x)  (sizeof(x) / sizeof((x)[0]))

#include "mstimer.c"
#include "task_structure.c"
#include "motor.c"
#include "adc.c"
#include "fifo_queue.c"

// ms timer counter
uint32_t time_ms = 0;

// flags and parameter for terminal logging
char enableLogging = 0;
int16_t nLogs = 70;		// number of log entries to print
int16_t logCounts = 0;

// flags to indeicate whether the motor has reached its reference position
int arrivalFlag = 0;

// interpolator to store trajectories. To be initilized in main().
List* interpolatorQueue;

// Motor data
int16_t TOP=60;
int16_t torque=0;
int16_t raw_torque=0;
int16_t kp_pot=0;
int16_t kp_ui=45;
int16_t kp_total=45; //tuned ideal kp_total=45
int16_t kd=5;		//tuned ideal kd=5
int16_t P=0; // error
int16_t P_last=0;
int16_t Pm_last=0;
int16_t dP=0;
int16_t Pr=2249;// Reference position - should be in encoder counts
int16_t Pm=0;  // Measured position - should be in encoder counts
int16_t Vm=0;
uint32_t last_ms = 0;
int16_t dt = 1;
int16_t direction = 1; // 1=forward, -1=backward
int16_t kpTerm = 0;
int16_t kdTerm = 0;

void PD_controller_task(){
	cli();
	// Get current position, Pm, and error, P
	//Pm = readEncoderInDegrees();
	Pm = readEncoderInCounts();
	P = (Pr - Pm);
	P_last = (Pr - Pm_last);

	// Compute velocity, Vm
	dP = ( P - P_last );
	dt = (time_ms - last_ms);
	Vm = dP/dt;

	// Compute torque
	kpTerm = (kp_total*(int32_t)P)/100; //kp_total = kp_pot + kp_ui; this value is updated by a schduler task. Scale down by a factor of 100 for more fine grain potentiometer control.
	kdTerm = (kd*dP)/dt;
	torque = kpTerm + kdTerm; 

	// Apply torque
	raw_torque = torque;
	if(torque < 0){
		setMotorBackward(motorPolarity);
		direction = -1;
		torque = abs(torque);
	}else{
		setMotorForward(motorPolarity);
		direction = 1;
	}
	// Pick the torque = MAX(torque, TOP)
	if (torque > TOP){
		torque = TOP;
	}
	updateDutyCycle(torque);

	// print
	printToTerminalIfLoggingEnabled();

	// Save for next frame
	Pm_last = Pm;
	last_ms = time_ms;
	sei();

}

int update_kp_task(){
	cli();
	kp_pot = adc_read_percent();
	kp_total = kp_pot + kp_ui;
	sei();
	return 1;
}

int UI_task(){
	USB_Mainloop_Handler();
	char c;
	if ((c = fgetc(stdin)) != EOF) {
		processUserInput(c);
	}
	return 1;
}

void constant_dutyCycle_task(){
    cli();
    enableTimer = 1;
	updateDutyCycle(25);
	sei();
}

int detect_arrival_task(){
	static arrivalFlag=0;

	// detect arrival
	static small_P_and_T_occurances;
	int threshold = 10;
	if(abs(P) < 5){
		if(abs(torque) < 3){
			small_P_and_T_occurances++;
		}
		// when the last 10 threshold of P<5 and toqure<3 occured, this is considered as arrival.
		if(small_P_and_T_occurances > threshold){
			arrivalFlag = 1;
		}
	}else{
		small_P_and_T_occurances = 0;
	}

	// hold for 10ms
	delay10ms();


	// release next task in interpolator
	if (arrivalFlag==1){
		arrivalFlag = 0;
		executeNextTrajectory(interpolatorQueue);
	}

}

void executeNextTrajectory(List* interpolatorQueue){
	cli();
	int16_t relative_reference;
	relative_reference = Q_pop_int16t(interpolatorQueue); // rel ref are in degrees
	if (relative_reference!=NULL){
		printf("Executing trajectory: %d\r\n", relative_reference);
		printInterpolatorQueue(interpolatorQueue);
		Pr = Pm + degreeToEncoderCounts(relative_reference); //Pr, Pm are in encoder counts
		enableLogging = 1;
		printLogHeader();
	}

	sei();
}

void delay10ms(){
	static uint32_t timerLast = 0;
	static uint32_t elapsed = 0;
	while (elapsed < 10){
		cli();
		elapsed = elapsed + (time_ms - timerLast);
		timerLast = time_ms;
		sei();
	}
	return 1;
}

void printLogHeader(){
	printf("kd\t");
	printf("kp\t");
	printf("Pr\t");
	printf("Pm\t");
	printf("P\t");
	printf("Pm_last\t");
	printf("P_last\t");
	printf("dP\t");
	printf("dt\t");
	printf("Vm\t");
	printf("Torque\t");
	printf("raw_T\t");
	printf("kpT\t");
	printf("kdT\t");
	printf("\r\n");
}

void printLogContent(){
	printf("%d\t", kd);	
	printf("%d\t", kp_total);	
	printf("%d\t", Pr);	
	printf("%d\t", Pm);
	printf("%d\t", P);
	printf("%d\t", Pm_last);
	printf("%d\t", P_last);
	printf("%d\t", dP);
	printf("%d\t", dt);
	printf("%d\t", Vm);
	printf("%d\t", torque*direction);
	printf("%d\t", raw_torque);
	printf("%d\t", kpTerm);
	printf("%d\t", kdTerm);
	printf("\r\n");	
}
void printToTerminalIfLoggingEnabled(){
	if (enableLogging==1){
		logCounts++;
		printLogContent();
	}
	if(logCounts > nLogs){
		logCounts = 0;
		enableLogging=0;
	}
}

int printCurrentData(){
	cli();
	printLogHeader();
	printLogContent();
	sei();	
}



void addTrajectoryTo(List* interpolatorQueue, int16_t trajectoryDegree){
	cli();
	TYPE type_t = INT16;
	Q_push(interpolatorQueue, &trajectoryDegree, sizeof(int16_t), type_t);
	printf("Added %d degrees to interpolator\r\n", trajectoryDegree);
	printInterpolatorQueue(interpolatorQueue);
	sei();
}



void processUserInput(char c){
	USB_Mainloop_Handler();
	int16_t relative_reference;
	switch(c){
		
		case 'Z': case 'z':
			printf("Z/z - resetting encoder\r\n");
			cli();
			Pr = 0;
			global_counts_m2 = 0;
			sei();
			break;

		case 'V': case 'v':
			printCurrentData();
			break;		
		
		case 't':
			executeNextTrajectory(interpolatorQueue);
			break;
		
		case 'P':
			if (kp_ui < 100){
				kp_ui++;
				update_kp_task();
				printf("kp_total updated to: %d\r\n", kp_total);
			}
			break;
		
		case 'p':
			if (kp_ui > 0){
				kp_ui--;
				printf("kp_total updated to: %d\r\n", kp_total);
			}
			break;
		
		case 'D':
			if (kd < 100){
				kd++;
				printf("kd updated to: %d\r\n", kd);
			}
			break;
		
		case 'd':
			if (kd > 1){
				kd--;
				printf("kd updated to: %d\r\n", kd);
			}
			break;

		case '1':
			addTrajectoryTo(interpolatorQueue, 90);
			break;

		case '2':
			addTrajectoryTo(interpolatorQueue, 180);
			break;

		case '3':
			addTrajectoryTo(interpolatorQueue, 360);
			break;
		
		case '4':
			addTrajectoryTo(interpolatorQueue, 720);
			break;

		case '5':
			addTrajectoryTo(interpolatorQueue, 5);
			break;

		case '0':
			addTrajectoryTo(interpolatorQueue, -90);
			break;

		case '9':
			addTrajectoryTo(interpolatorQueue, -180);
			break;

		case '8':
			addTrajectoryTo(interpolatorQueue, -360);
			break;

		case '7':
			addTrajectoryTo(interpolatorQueue, -720);
			break;

		default:
			break;
	}
}



// Task pointers defined in task_structure.c
// task functions such as PD_controller_task() are define near the top of this file.
void initAllTasks(){
	/**        task, function,           period, expT, missedDL, ID, priority, state, jobsInQ, qMaxSeen **/
	setUpATask(&t1,  PD_controller_task,  20,     0,    0,        1,  1,       1,     0,       0);
	setUpATask(&t2,  detect_arrival_task,200,     0,    0,        2,  2,       1,     0,       0);
	setUpATask(&t3,  update_kp_task,     200,     0,    0,        3,  3,       1,     0,       0);
	setUpATask(&t4,  UI_task,            300,     0,    0,        4,  4,       1,     0,       0);
	//setUpATask(&t1,  constant_dutyCycle_task,     20,     0,    0,        1,  1,       1,     0,       0); // find this task in motor.c
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

// Power on pin 7 - use to power potentiometer or for general testing.
void powerOnPE6(){
    DDRE |= (1 << PORTE6);
    PORTE |= (1 << PORTE6);
}

int main() {

	cli();

	// initialization routine for the serial communication
	sanityCheck(); // sanity comes first because the LED setup changes push button PCINT registers used by other codes
	SetupHardware();

	// Setup motor
	setupMotor2();
	setupEncoder();
	calibrateMotorPolarity(); // motorPolarity=1 means encoder counts up with motorForward(). motorPolarity=0 means encoder counts down with motorForward()

	// Setup ADC
	adc_init();
	powerOnPE6(); // Use pin 7 to power potentiometer

	// ms timer
	timer3_mstimer_init();
	initAllTasks();

	
	// Main loop
	sei();
	global_counts_m2 = 0;
	interpolatorQueue = getDoublyLinkedList();
	while(1){
		runHighestPriorityReadyTask(tArrSorted, LENGTH(tArrSorted));
    }
	return 0;
}



// This is the interrupt service routine for timer3 
ISR(TIMER3_COMPA_vect){
	// Increment ms
	time_ms++;

	// For each task, check if any task is ready to be released
	refreshTasksReadiness(tArrSorted, LENGTH(tArrSorted), time_ms);
}
