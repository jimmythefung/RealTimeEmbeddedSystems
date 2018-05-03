### RTES Scheduling Lab: Counters, Timers, and Scheduling
### Due :  Saturday, March 31

#### Introduction

The purpose of this lab is to analyze an embedded system with respect to its timing requirements, constraints, and characteristics. The program you will experiment with will execute (schedule) several tasks using the various programming constructs that we have been working on over the past few weeks. The MOST IMPORTANT part of this assignment is understanding the behavior of the system. Please answer the questions at the bottom of the document thoughtfully, and demonstrating your depth of understanding.

There are several pieces to this assignment. Please read this document carefully to ensure you understand what is being asked of the system and of you.

#### System Analysis

Use dynamic analysis to determine the WCET of each of these tasks. Develop a periodic task model such that you define the w(t) equation for each task. This includes blocking due to nonpreemption.

#### The System Tasks

You will be given an almost complete program that manages the tasks described below using the scheduler from the previous lab<sup>[1]</sup>. Implement these in a way that is as precise as possible with respect to the specified timing requirements. Once this system is complete, you will add delays to disrupt the timing of these components, thereby making it impossible to meet some of the timing requirements.

**__Scheduler Task__**: Use a timer/counter interrupt to track time in ms increments and to set the status of tasks.

**__RED LED Task__**: Toggle the GPIO Red LED at a default rate of 10 Hz (change state every 100 ms). Make this the highest priority task.

**__YELLOW LED Task__**: Toggle the GPIO Yellow LED at a default rate of 10 Hz (change state every 100 ms). This task is executed from inside an ISR (not the one used for the scheduler) with a frequency of 40Hz. Note that the frequency of the ISR is greater than the frequency of the toggle rate.

**__EVENT POLLING TASK__**: Periodically poll for a change in value of the potentiometer from the last time that it was read. This event should then set a flag that the scheduler will check to release the Semaphore Task. This follows the logic of a semaphore in which one task will wait for the signal of another to execute.

**__SEMAPHORE TASK__**: Turn on the yellow on-board LED for a fixed amount of time in response to the “event” which is monitored by the EVENT POLLING TASK. When this task is waiting for that event, it is blocking. When the task is released, turn on the on-board Yellow LED, delay for 5ms, then turn it off. The intent is that those 5ms represent use of the CPU, thus the delay should be implemented as a busy-wait.

**__Hough Transform Task__**: Execute the Hough Transform on the provided image segment from within the main loop. Run the transform over the image segment multiple times so that it takes as close to 50ms as possible to execute this task, without going UNDER 50ms. Determine how many times to transform the image segment using WCET analysis. Release this task from within a timer/counter ISR at 10Hz. (Note that it is not possible to store an entire image in memory, thus if your system had to analyze real-time video, it would either have to be transferred in small segments or preprocessed prior to transferring to the microprocessor. This Hough Transform Task simulates a computationally intensive task such as real-time image analysis.)

>The Hough Transform code serves multiple purposes in this assignment: 1) it gives you experience with calculating WCET, 2) demonstrates how to use PROGMEM, 3) demonstrates how constrained the resources are, and 4) makes the computationally intensive task more real than spinning in a busy-wait. The functionality of the code is not meaningful, thus use it to fulfill the purposes listed above.

**__GREEN LED Task__**: Toggle the PWM Green LED at a default rate of 10 Hz (change state every 100 ms). Allow the user to modify the frequency of this task through the menu. This task is implemented using a PWM signal.

**__GREEN LED Counting Task__**: Maintain a count of the number of toggles of the GREEN LED. You can use the same timer/counter used to generate the PWM signal.

**__Communication Task__**: Upon the release of button A, pause the system and prompt the user with some basic menu options to either reconfigure the system or print out some data. The menu options are provided below. When the user is done, resume or reset the system, as appropriate. Remember that you can set timer/counters to 0 to start the count over.

> The Communication Task is a means for us to modify the system and report results. It should not be included as part of the overall analysis, nor does it have a deadline.

<sup>[1]</sup>You develop the HOUGH, SEMAPHORE, and EVENT POLLING TASKS.


### The System Hardware

If you place your LEDs at different locations, please make your code flexible enough that the TA can easily change your code to these port pins.
- Green : Port B, pin 6.
- Yellow: Port D, pin 6.
- Red: Port B, pin 4.

### The System Menu

Use the following menu options:
- p : Print data collected for experiment, including job releases and missed deadlines.
- e # : Set-Up this experiment number (e.g. Set flags to activate/deactivate delays in ISRs).
- r # : Set the toggle period of the GREEN LED Task to # ms. (Check that value is in bound.)
- z : Reset all variables to set up for a new experiment.
- g : Go signal for start experiment. (Try to synchronize all activities when this signal is given).

Examples:
```
r 2000 : Set the GREEN LED Task to toggle every 2000 ms.
e 2 : Set up experiment 2.
```

You can add any additional menu options that make it easy for you to develop, debug, and experiment with your system. Please provide a description of these commands in a readme file.

### System Data Collection

For each task, keep track of the number of times a job is executed and the number of missed deadlines. A missed deadline would be one in which the next job was released. For example, if the Hough Transform Task was released but it was still executing, this would be a missed deadline. Note that no task should miss its deadline, except maybe the RED LED Task, when the system is first configured without any delay loops.

For some tasks, you can use a counter instead of a flag, in which the ISR increments the counter and the task decrements the counter. If the count exceeds 1, it indicates a missed deadline. For other tasks, tracking missed deadlines is far more difficult and even impossible without additional hardware. At the most rudimentary level, one can run an experiment for a known length of time, know what the toggle count should be and compare that to the actual count to determine the missed deadlines.

### Code Assessment

Logically correct, meets functional requirements, well organized, good naming conventions, and commented, but not superfluously.

### Program Execution

1. Turn on all GPIO LEDs at the start of the program, delay, and turn off to confirm LEDs are functional.
2. Set-up all the appropriate components of your system.
3. Print out the basic menu options to the user.
4. A “go” prompt from the user should set timer/counters to 0, enable interrupts, and enter the main loop.


### Report:

The intent of this lab is to demonstrate the mechanisms by which you can “schedule” on a microprocessor and to understand the advantages and disadvantages of those techniques. The delays that are being inserted into the system are impacting the performance and are meant to help you understand the impact of bad design decisions (like delays in ISRs), as well as understand the impact of spurious disruption to the system and which techniques are more robust in the face of that disruption.

As the delays increase, it will become more difficult to gather and report missed deadlines and toggle counts. Do the best that you can, and when you arrive at a situation in which your results are unreliable (or unattainable), state that in your report and explain why.


### Experiments

Run a series of experiments as described below then answer the questions. For each experiment,
- Zero all job release and missed deadline counters.
- Configure the system for the experiment.
- Run the experiment for approximately 15 seconds.
- Record the number of releases and missed deadlines for all LEDs.

1. Configure the system so that all LEDs except the “semaphore” LED, toggle at a frequency of 2 Hz (I set the frequency such that you could better see what is going on. If you want to run it slower or faster, feel free.) Run the system for about 30 seconds to confirm everything is functional and meeting the time requirements. This means that everything but the yellow on-board LED should be blinking in sync, at least at the start of the experiment.

2. Configure the system to the default frequencies (mostly 10 Hz). Place a 20ms busy-wait delay in the ISR for the GREEN LED Counting Task. Place it after the count. Run experiment and record results.

3. Place a 20ms busy-wait delay in the ISR for the YELLOW LED Task (remove the delay from #2 above). Place it after the toggle. Run experiment and record results.

4. Repeat #2, except use a 30ms busy-wait.
5. Repeat #3, except use a 30ms busy-wait.

6. Repeat #2, except use a 105ms busy-wait.
7. Repeat #3, except use a 105ms busy-wait.

8.
  Repeat #7 (i.e. 10Hz toggle with 105ms busy-wait in YELLOW LED Task), except place an sei() at the top of the ISR before the toggle and delay.



#### Answer the Following Questions and Submit with Code

**__The most important part of this assignment is understanding the behavior of your system. Please answer the questions thoughtfully and demonstrating your depth of understanding. Large quantities of text does not equate to depth of understanding.__**

1. Describe your method for WCET analysis. How confident are you in those results? Briefly explain. What factors influence the accuracy of a WCET analysis?

2. Define the w(t) equation for each of the tasks.

3. In experiment #1,
  a. Did you observe any “drift” in the blinking of the LEDs, meaning they were in sync but then one seemed to blink slightly slower or faster than the other? Briefly explain how drift can occur.
  b. Were the LEDs synchronized at the start, meaning were they all on, then all off at the exact same time? Describe the factors that influence the ability to synchronize these events.

4. Using the data collected in the above experiments, describe the behavior for each of the experiments #2 through #7, including number of job releases, missed deadlines, and expected number of releases. Explain the reason for the behavior for each of the experiments #2 through #7 (you can discuss each individually or describe them collectively).

5. Using the data collected in experiment #8, describe and explain the behavior of the system.

6. Consider the various scheduling methods used here and discussed in class. For each method below, discuss the control over the timing of that task, responsiveness of the system, and the impact on other tasks with respect to timing.
  a. Time-driven execution inside an ISR (e.g. the scheduler),
  b. Time-driven release managed by the scheduler,
  c. External interrupt with execution inside an ISR (e.g. Communication Task, if inside ISR),
  d. Periodic polling for an event that releases another task (e.g. Polling and Semaphore tasks).
