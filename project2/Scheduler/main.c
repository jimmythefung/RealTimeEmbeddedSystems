#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Uncomment this to print out debugging statements.
//#define DEBUG 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lufa.h"
#include "common.h"
#include "leds.h"
#include "timers.h"
#include "buttons.h"
#include "event_polling.h"
#include "semaphore.h"
#include "hough_task.h"
//#include "../hough/hough.h"

#include "red_gpio.h"
#include "yellow_gpio.h"
#include "green_gpio.h"

// tri-state system
// Blocking (waiting for release). Ready (waiting for cpu). Running (has cpu)
#define BLOCKING 0
#define READY 1
#define RUNNING 2

// setting caps on priority and number of spawned tasks
#define MAX_PRIORITY 250
#define MAX_TASKS 10

// experiment number impacts task frequency and ISR delays in yellow and green
volatile int experiment = -1;

char set_up_experiment = 0;

// experiment expire timer
volatile uint64_t experiment_expire_time;

// used in the TIMER0_COMPA ISR
volatile uint64_t ms_ticks;

// mutex access to ms_ticks
uint64_t get_ticks() {
  uint64_t temp;
  cli();
  temp = ms_ticks;
  sei();
  return temp;
}

// Used to pause scheduler while printing stuff out
char paused = 0;

char in_ui_mode = 0;
char in_experiment_mode = 0;

/****************************************************************************
   TASK Data Structures
****************************************************************************/
// holds a task. All will be gathered into an array
typedef struct {
	int (*funptr)();
  int period; 						// milliseconds
  uint64_t next_release;  // absolute time of next release in ms
  int missed_deadlines;
  char id;
  int priority; 		// priority 1 has the highest priority
	int buffered;			// the number of jobs waiting to execute
  int max_buffered; // maxim bufferend while system running
  int releases;     // number of times released
  int executed;     // number of times executed
  int state;				// one of the 3 states
} Task;

// shared structure between scheduler (ISR) and server (main)
volatile Task tasks[MAX_TASKS];

// Array is initially empty. Spawn tasks to add to scheduler.
int task_count = 0;

/****************************************************************************
   TASK Scheduling Functions
****************************************************************************/
/* The creation of a single task, which is added to the array of tasks
 * Assuming all tasks in phase and will be released at start of system.
 * param [in] funptr : the code to be executed for a single job of the task.
 * param [in] id : not used in this version, but might have future utility
 * param [in] p : (period) time between releases in ms
 * param [in] priority : a fixed priority system (probably RM)
 */
int spawn(int(*fp)(), int id, int p, int priority) {
	if (task_count == MAX_TASKS) {
		return ERROR;
	}
  tasks[task_count].funptr = fp;
  tasks[task_count].period = p;
  tasks[task_count].next_release = p;
  tasks[task_count].id = id;
  tasks[task_count].priority = priority;
	tasks[task_count].buffered = 1;
  tasks[task_count].max_buffered = 1;
  tasks[task_count].releases = 0;
  tasks[task_count].executed = 0;
  tasks[task_count].state = READY;
  ++task_count;
	return 1;
}

// A fixed priority system based on the period of a task
// The smaller the period, the higher the priority.
// // setting caps on priority and number of spawned tasks
#define RED_GPIO_TID 1
#define E_POLLING_TID 2
#define SEMAPHORE_TID 3
#define HOUGH_TID 4
void spawn_all_tasks() {
	// @TODO: confirm that all tasks are spawned without error.
  // spawn(fptr, id, period, priority)
  //    fptr                 id             period             priority 
  spawn(&red_gpio_task,      RED_GPIO_TID,  red_gpio_period,   1);
  spawn(&event_polling_task, E_POLLING_TID, 100,               2);
  spawn(&semaphore_task,     SEMAPHORE_TID, 100,               3);
  spawn(&hough_task,         HOUGH_TID,     100,               4);
}


// Prints the execution time of a given function, fp().
uint64_t fp_execution_mstimer(int(*fp)(), int nTrials){
  uint64_t start_ms;
  uint64_t delta;
  int i = 0;
  
  // start timer
  cli();
  start_ms = ms_ticks;
  sei();

  // run function nTrials times
  for(i=0; i<nTrials; i++){
    fp();
  }

  // end timer, compute average time elapsed
  cli();
  delta = ms_ticks - start_ms;
  sei();

  return delta;

}


void print_fp_execution(int(*fp)(), uint16_t nTrials, char* printstring){
  uint16_t delta;
  uint16_t avg;

  // get total run time in ms
  delta = fp_execution_mstimer(fp, nTrials);

  // compute avg
  cli();
  avg = delta / nTrials;
  sei();

  // print
  USB_Mainloop_Handler();
  cli();
  printf("%s WCET: ms=%u, nTrials=%u, avg=%u  \r\n", printstring, delta, nTrials, avg);
  sei();
}

/****************************************************************************
   ALL INITIALIZATION
****************************************************************************/
void initialize_system(void)
{
	initialize_leds();
	light_show();

  initialize_buttons();

  initialize_red_gpio();
  initialize_yellow_gpio();
  initialize_green_gpio();

	spawn_all_tasks();

	SetupHardware();

  // setup adc to read from potentiomenter
  adc_init();
  powerOnPE6(); // use Pin7 (PortE6) to power potentiomenter

	// SCHEDULER: timer 0, prescaler 64, period 1 ms
	SetUpTimerCTC(0, 64, 1);
}

void ReleaseA() {
  // toggle the green to confirm button release recognized
  PORTD &= ~(1<<PORTD5);
  _delay_ms(100);
  PORTD |= (1<<PORTD5);
  in_ui_mode = 1;
  experiment = -1;
  cli();
}

/****************************************************************************
   MAIN
****************************************************************************/
#ifdef DEBUG
volatile char print_flag = 0;
volatile char task_state = -1;
int execute_count = 0;
#endif

int main(void) {
  // This prevents the need to reset after flashing
  USBCON = 0;

  int flag = 1;  // used to print results only upon first release USB handler

  int task_id = -1;
	int highest = MAX_PRIORITY + 1;

  // Do not include serial comm in tasks for purposes of experiment.
  uint64_t handler_next_release = 60000;
  int handler_period = 60000;

	initialize_system();

  #ifdef DEBUG
  uint64_t loop_count = 0;
  #endif

  int temp;
  int i;

  // used to toggle led to show liveness
  int period_ms = 1000;
  uint64_t expires = period_ms;

  // Set up to fire ISR upon button A activity
  // Upon the release (2nd input param) of the button, it will call fn ReleaseA
  SetUpButton(&_button_A, 1, ReleaseA );

  // HERE WE GO
  ms_ticks = 0;
  timer3_ticks = 0;
  sei();

  char c;

  //*******         THE CYCLIC CONTROL LOOP            **********//
  //*************************************************************//
  while(1) {
    if (in_ui_mode) {
      cli();
      USB_Mainloop_Handler();
      if ((c = fgetc(stdin)) != EOF) {
        handleInput(c);
      }
      continue;
    } // end if in_ui_mode

    /*
    // heartbeat
    if (get_ticks() >= expires) {
      TOGGLE_BIT(*(&_yellow)->port, _yellow.pin);
      expires = get_ticks() + period_ms;
    }
    */

    #ifdef DEBUG
    ++loop_count;
    #endif

    // Determine highest priority ready task
    task_id = -1;

    #ifdef DEBUG
    printf("%d: task id %d --- ",(int)loop_count, task_id);
    USB_Mainloop_Handler();
    #endif

    highest = MAX_PRIORITY + 1;
    for (i = 0; i < task_count; i++) {
      cli();
      temp = tasks[i].state;
      sei();
			if (temp == READY) {
				if (tasks[i].priority < highest) {
          task_id = i;
          highest = tasks[i].priority;
        }
      }
    } // end for i in tasks

    #ifdef DEBUG
    if (print_flag) {
      printf("ready but %d\r\n", task_state);
      USB_Mainloop_Handler();
      print_flag = 0;
    }
    printf(" %d \r\n",task_id);
    USB_Mainloop_Handler();
    #endif

		// Execute the task, then do housekeeping in the task array
		if (-1 != task_id) {
      cli();
      tasks[task_id].state = RUNNING;
      sei();
			tasks[task_id].funptr();

      cli();
      tasks[task_id].executed += 1;
			tasks[task_id].buffered -= 1;
			if (tasks[task_id].buffered > 0) {
				tasks[task_id].state = READY;
			} else {
				tasks[task_id].state = BLOCKING;
			}
      sei();
		} // end if -1 != task_id

    #ifdef DEBUG
    if (print_flag) {
      printf("ready but %d\r\n", task_state);
      USB_Mainloop_Handler();
      print_flag = 0;
    }
    _delay_ms(1);
    #endif

    // find out the execution time of scheduler tasks; print_fp_execution(fp, nTrials, printstring) 
    // print_fp_execution(&event_polling_task, 3000, "Event Polling Task"); // 0.042ms
    // print_fp_execution(&semaphore_task, 50, "Semaphore Task");     // 5.1ms
    // print_fp_execution(&hough_task, 5, "Hough Task"); // 52ms (using 6x6 image10.h). If using 5x5, and it takes 28ms.
    // print_fp_execution(&green_gpio_task, 30000, "Green LED Task");
    // print_fp_execution(&yellow_gpio_task, 30000, "Yellow LED Task");
    // print_fp_execution(&red_gpio_task, 30000, "Red LED Task");




    /*
		// Alternative approach ...
		// If the tasks are in the task table in order of priority,
		// then finding the first one that is ready guarantees it is the highest
		for (int i = 0; i<task_count; i++) {
			if (tasks[i].state == READY) {
				tasks[i].state = RUNNING;
				tasks[i].funptr();
				tasks[i].state = BLOCKING;
				// It is important to break or you won't be looking at priority.
				break;
			} // end if
		} // end for i in tasks
    */

    /*
    if (get_ticks() >= handler_next_release) {
      if (flag) {
          paused = 1; // pause the ms_ticks without disabling interrupts
        for (i=0;i<task_count;i++) {
          printf("ID:%d Released:%d Executed: %d Missed:%d MaxBuffered:%d \r\n",
          tasks[i].id, tasks[i].releases, tasks[i].executed, tasks[i].missed_deadlines, tasks[i].max_buffered);
          USB_Mainloop_Handler();
        }
        handler_next_release = get_ticks() + handler_period;
        paused = 0;
        flag = 0;
      }
    } // end if check handler_next_release
    */
  } /* end while(1) loop */
} /* end main() */

/****************************************************************************
   ISR for TIMER used as SCHEDULER
****************************************************************************/
// Timer set up in timers.c always enables COMPA
ISR(TIMER0_COMPA_vect) {

  if (paused) return;

	// Find all tasks that are ready to release.
	// Determine if they have missed their deadline.
	int i;
  // Check the task table to see if anything is ready to release
	for (i = 0; i < task_count; i++) {

		if (tasks[i].next_release == ms_ticks) {
      tasks[i].releases += 1;

      #ifdef DEBUG
      task_state = tasks[i].state;
      print_flag = 1;
      #endif

			if (tasks[i].state != BLOCKING) {
				tasks[i].missed_deadlines++;
			}
      tasks[i].state = READY;
			tasks[i].next_release += tasks[i].period;
			tasks[i].buffered += 1;
      if (tasks[i].buffered > tasks[i].max_buffered) {
        tasks[i].max_buffered = tasks[i].buffered;
      }
    }

    // Correction for special case - semaphore task: set back to blocking if event_poll_flag is 0.
    if (tasks[i].id==SEMAPHORE_TID && event_poll_flag==0){
      tasks[i].state = BLOCKING;
    }

  }
  // ms_ticks is down here because want all tasks to release at 0 ticks
  ++ms_ticks;

  if (in_experiment_mode) {
    if (experiment_expire_time == ms_ticks) {
      in_experiment_mode = 0;
      PORTD &= ~(1<<PORTD5);
    }
  } else if (set_up_experiment) {
    experiment_expire_time = ms_ticks + 15000;
    in_experiment_mode = 1;
    set_up_experiment = 0;
  }
}
