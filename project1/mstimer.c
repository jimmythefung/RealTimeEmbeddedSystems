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