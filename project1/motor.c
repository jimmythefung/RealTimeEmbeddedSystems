/*
 * basic_motor.c
 *
 * Created: 02/2017
 * Author : Chris Owens and Amy Larson
 */
void setupMotor2(void);
void setupEncoder(void);
void setDutyCycle( uint16_t);

// comment out line if not debugging
#define DEBUG_PCINT

// For setting frequency of timer. Freq calculation based on TOP+1
#define TOP_4kHz 3999

// Motor 2 PWM signal generated on B6
#define m2_port PORTB
#define m2_pin PORTB6
#define m2_control DDRB

// The direction of motor 2 is controlled on Port E pin 2
#define m2_dir_control DDRE
#define m2_dir_pin DDE2
#define m2_dir_output PORTE

// Set the direction (arbitrarily) to 0.
// Whether this generates positive or negative encoder counts depends on how motor is plugged in.
#define motorForward() clearBit( m2_dir_output, m2_dir_pin )
#define motorBackward() setBit( m2_dir_output, m2_dir_pin )

// Turn motor on and off by changing direction of signal to output (set) or input (clear)
#define OnMotor2() setBit( m2_control, m2_pin )
#define OffMotor2() clearBit( m2_control, m2_pin )

// Encoder channels (yellow and white wires) must be plugged in to PB4 and PB5 (Astar pins 8 and 9)
// You can plug in elsewhere, but then this code needs changing.
#define chA_control DDRB
#define chA_pin DDB4
#define chB_control DDRB
#define chB_pin DDB5
// Interrupt #s for encoder are based on where they are plugged in. See AStar pinout PB4 = PCINT4
#define chA_INT PCINT4
#define chB_INT PCINT5

// Powering the encoder through general I/O. Plug power (blue) in to Port D Pin 1 (AStar pin 2!!)
// You can put these anywhere you like, but moving blue means changing this code.
// Plug green into any ground in general I/O.
#define enc_power_control DDRD
#define enc_power_pin DDD1
#define enc_power_output PORTD

/* PCINT for Encoder was ported from
*  PololuWheelEncoders.cpp
*/
volatile int8_t global_m2a;
volatile int8_t global_m2b;

volatile int16_t global_counts_m2;

volatile int8_t global_error_m2;

volatile int16_t global_last_m2a_val;
volatile int16_t global_last_m2b_val;

#ifdef DEBUG_PCINT
	volatile uint32_t interrupt_counter = 0;
#endif

void setupMotor2(void) {

	// Make sure motor is not going to start until all set up
	OffMotor2();

	// Configure the motor direction pin to output
	setBit( m2_dir_control, m2_dir_pin );

	// start off going "forward" with the intent of increased encoder counts
	motorForward();

	// Motor 2 is connected to Timer0 Channel B
	//Clear the registers
	TCCR1A = 0;
	TCCR1B = 0;
	ICR1 = 0;

	// Clearing on match for non-inverted signal (i.e. larger match value produces larger duty cycle)
	TCCR1A |= (1 << COM1B1);

	/* want mode 14 (1110), ICR1 is top, OCR1B is match */
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	// Using 1 as prescaler (001)
	/* 4kHz counter frequency, period of 1/4000 = .25ms period */
	TCCR1B |= (1 << CS10);
	ICR1 = TOP_4kHz;
}

void setupEncoder(void) {

	// Set the encoders as input
	clearBit( chA_control, chA_pin );
	clearBit( chB_control, chB_pin );

	// Enable the interrupts for the 2 encoder channels
	setBit( PCMSK0, chA_INT );
	setBit( PCMSK0, chB_INT );

	// enable PCINT interrupts
	setBit( PCICR, PCIE0 );

	// Powering the encoder through general I/O. This sets signal high to provide power to device.
	setBit( enc_power_control, enc_power_pin );  // Set direction as output
	setBit( enc_power_output , enc_power_pin );  // set port value to high
}


int16_t enableTimer=0;
uint32_t encAvgFrq = 0;
uint32_t encoderTime = 0;
uint32_t encoderCount = 0;
uint32_t count_limit = 50000;
uint32_t time_ms;
uint32_t encoderLastms = 0;

void timeTheEncoder(){
	cli();
	if (enableTimer == 1){
		if(encoderCount < count_limit){
			encoderTime = encoderTime + (time_ms - encoderLastms);
			encoderCount++;
			if (encoderCount == count_limit){
				encAvgFrq = (encoderCount * 1000) / encoderTime;
			}
		}
	}
	encoderLastms = time_ms;	
	sei();
}

ISR(PCINT0_vect){
	timeTheEncoder();

	#ifdef DEBUG
		++interrupt_counter;
	#endif

	// Make a copy of the current reading from the encoders
	uint8_t tmpB = PINB;

	// Get value of each channel, making it either a 0 or 1 valued integer
	uint8_t m2a_val = (tmpB & (1 << chA_pin )) >> chA_pin;
	uint8_t m2b_val = (tmpB & (1 << chB_pin )) >> chB_pin;

	// Adding or subtracting counts is determined by how these change between interrupts
	int8_t plus_m2 = m2a_val ^ global_last_m2b_val;
	int8_t minus_m2 = m2b_val ^ global_last_m2a_val;

	// Add or subtract encoder count as appropriate
	if(plus_m2) { global_counts_m2 += 1; }
	if(minus_m2) { global_counts_m2 -= 1; }

	// If both values changed, something went wrong - probably missed a reading
	if(m2a_val != global_last_m2a_val && m2b_val != global_last_m2b_val) {
		global_error_m2 = 1;
	}

	// Save for next interrupt
	global_last_m2a_val = m2a_val;
	global_last_m2b_val = m2b_val;

	// If trying to debug, flash an led so you know the PCINT ISR fired
	#ifdef DEBUG_PCINT
		if (0 == interrupt_counter%20 ) {
			toggleBit( PORTD, PORTD5 );
		}
	#endif
}

/* set duty cycle, duty is a percent of full speed */
void setDutyCycle( uint16_t duty ) {
	if ( duty == 0 ) {
		OCR1B = 0;
		OffMotor2();
		return;
	}

	if (duty > 100) {
		duty = 100;
	}
	OCR1B = (uint16_t) ((TOP_4kHz + 1.0) * (duty / 100.0 )) - 1;
	OnMotor2();
}


/* 
 * Determines how the user hook up the postive and negative terminal of the motor.
 * The polarity result is used to determine "forward" as encoder counting up and "backward" as counting down.
 */
volatile char motorPolarity;
void calibrateMotorPolarity(){
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

	motorPolarity = polarity; //Polarity=1 means motorForward() counts up
}

void setMotorForward(char motorPolarity){
	if (motorPolarity==1){
		motorForward();
	}else{
		motorBackward();
	}
}

void setMotorBackward(char motorPolarity){
	if (motorPolarity==1){
		motorBackward();
	}else{
		motorForward();
	}
}

void updateDutyCycle( uint16_t duty ){
	if(duty < 1){
		setDutyCycle(0);
	}else{
		setDutyCycle(duty);
	}
}

int16_t readEncoderInDegrees(){
	int16_t degree = 360* (int32_t)global_counts_m2 / 2249;
	return degree;
}

int16_t degreeToEncoderCounts(int16_t degree){
	int16_t counts = (int32_t)degree*2249/360;
	return counts;
}

int16_t encoderCountsToDegree(int16_t encoderCounts){
	int16_t degree = 360* (int32_t)encoderCounts / 2249;
	return degree;
}

int16_t readEncoderInCounts(){
	return global_counts_m2;
}