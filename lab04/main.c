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
#include<stdbool.h>
#include <stdio.h>
#include <VirtualSerial.h>

uint32_t time_ms = 0;
char yellowOn = 0;
volatile int counter2hz = 0;
volatile int brightnessLevel=5;
volatile int blinkSpeed = 5;
volatile int TOP=0xFF;

void setLEDOutput(){
	// Data Direction Register determines input/output modes
	DDRC |= (1 << DDC7); // Yellow; PortC; Pin7
	DDRD |= (1 << DDD5); // Green;  PortD; Pin5

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
}

void timer3_mstimer_init(){
	// Setup timer3 channel A using pre-scaler 64.
	TCCR3B |= (1<<CS30);  // only care about register B
	TCCR3B |= (1<<CS31);
	TCCR3B |= (1<<WGM32); // Using CTC mode with OCR1A for TOP. This is mode 4.; WGM(n)2 where n is timer 

	// Software Clock Interrurpt frequency: 1000 = f_IO / (prescaler*OCR1A)
	OCR3A = 250; //See p.122 When TCNT==OCRnA, sets TIFR (Timer Interrupt Flag Register), trigger TIMER3_COMPA_vect

	// Enable output compare match interrupt on timer 3 channel A
	TIMSK3 |= (1 << OCIE3A);
}

void timer1_pwm_blink_init(){
	// Setup timer 1 pwm using pre-scaler ??
	// Mode 14; WGM33=1, WGM32=1, WGM31=1, WGM30=0; see page 132
	TCCR1A |= (1<<WGM11);
	TCCR1B |= (1<<WGM12);
	TCCR1B |= (1<<WGM13);

	// Set COM (compare output mode) to clear OCnA on compare match: A1=1, A0=0
	TCCR1A |= (1<<COM1A1);

	// Set COM (compare output mode) to clear OCnB on compare match: B1=1, B0=0
	TCCR1A |= (1<<COM1B1);

	// Prescaler = 1024 = 2^10
	TCCR1B |= (1<<CS10);
	TCCR1B |= (1<<CS12);

	// Top = ICR1 = 2^16 (full width of counter register)
	blinkSpeed = 9;
	ICR1 = 0xFFFF - (0xFFFF/10)*blinkSpeed; // when blinkSpeed = 0, ICR1 = 0xFFFF; when blinkSpeed = 10, ICR1=0

	// Set default duty cycle to 50%: is half of TOP
	OCR1A = ICR1/2;
}

void timer0_pwm_brightness_init(){
	// Setup timer0 in mode 3 pwm using prescaler = 256	
	TCCR0A |= (1<<WGM01);
	TCCR0A |= (1<<WGM00);

	// Set COM (compare output mode) to clear OC0A on compare match p.104
	TCCR0A |= (1<<COM0A1);

	// Prescaler = 64
	TCCR0B |= (1<<CS01);
	TCCR0B |= (1<<CS00);

	// TOP is 0xFF
	TOP = 0xFF;
	brightnessLevel = 5;
	OCR0A = (TOP/10) * brightnessLevel; // Compare Output Register 1 Channel A match. Page 123
}

void setPortB5Output(){
	DDRB |= (1<<DDB5);
}
void setPortB7Output(){
	DDRB |= (1<<DDB7);
}

int main() {
	setLEDOutput();    // Set Yellow and Green LED as output
	setButtonInput();  // Set buttons A and B as interrupt
	setPortB5Output(); // Set OC1A (port B5) - timer 1
	setPortB7Output(); // Set OC0A (port B7) - timer 0
	timer3_mstimer_init(); 	// 1 kHz timer 
	timer1_pwm_blink_init();// pwm for LED blink - port B5
	timer0_pwm_brightness_init(); //pwm for LED brightness - port B7
	SetupHardware(); // This sets up the USB hardware and stdio
	sanityCheck();   // Blink the LEDs

	// Main loop
	sei();
	volatile char c;
	while(1){
		USB_Mainloop_Handler();

        if ((c=fgetc(stdin)) != EOF) {
        	// Use LUFA to interact. Screen shows usage instructions
        	printf("Usage: b=birghter, d=dimmer, f=faster, s=slower.\r\n");

            // Brightness
            if (c=='b'){
	        	if (brightnessLevel<10){
	        		brightnessLevel++;
	        	}
	        	printf("Brightness=%i%%\r\n", brightnessLevel*10);
	        	OCR0A = (TOP/10) * brightnessLevel;
            }

            if (c=='d'){
	        	if (brightnessLevel>0){
	        		brightnessLevel--;
	        	}
	        	printf("Brightness=%i%%\r\n", brightnessLevel*10);
	        	OCR0A = (TOP/10) * brightnessLevel;
            }

	     
	        // Blink
	        if (c=='f'){
	        	if (blinkSpeed<10){
	        		blinkSpeed+=1;
	        	}
	        	printf("Speed Level=%i\r\n", blinkSpeed);
	        	ICR1 = 0xFFFF - (0xFFFF/10)*blinkSpeed;
	        	OCR1A = ICR1/2; 
	        }

	        if (c=='s'){
	        	if (blinkSpeed>0){
	        		blinkSpeed-=1;
	        	}
	        	printf("Speed Level=%i\r\n", blinkSpeed);
	        	ICR1 = 0xFFFF - (0xFFFF/10)*blinkSpeed;
	        	OCR1A = ICR1/2; 
	        } 
        }

        // Blink LED at .5 Hz
        cli();
        if (time_ms>1000){
        	PORTD ^= (1 << PORTD5); // toggle green
        	time_ms=0;
        }
        sei();
    }
	return 0;
}


// This is the interrupt service routine for timer3
ISR(TIMER3_COMPA_vect){
	time_ms++;

	// toggle yellow at 2Hz
	counter2hz++;
	if (counter2hz > 250){
		PORTC ^= (1 << PORTC7);
		counter2hz = 0;
	}
}