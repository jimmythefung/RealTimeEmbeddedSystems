// This is the global flag inspected by both the semaphore task and event polling task to turn on the on board yellow led.
uint16_t event_poll_flag = 0;

// initialize adc
void adc_init() {
    // AREF = AVcc
    ADMUX = (1<<REFS0);

    // ADC Enable and prescaler of 128; 16 MHz/128 = 125kHz
    ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    // Define analog signal input form ADC8 (pin 4, port D4)
    DIDR2 |= (1 << ADC8D); // disable digital input to use analog input

    // Use pin 4 to read analog
    ADCSRB |= (1 << MUX5);

    // Set ADC mode: free-running
    ADCSRA |= (1<<ADATE);

    // Start ADC
    ADCSRA |= (1<<ADEN);
    ADCSRA |= (1<<ADSC);
}

// read adc value
uint16_t adc_read() {
	uint16_t adc_results = ADC;
	return adc_results;
}

uint16_t adc_read_percent(){
	uint16_t reading = adc_read();
	
	uint16_t percent = 100*(uint32_t)reading/1023 ;
	return percent;
}

void powerOnPE6(){
    DDRE |= (1 << PORTE6);
    PORTE |= (1 << PORTE6);
}

void read_pot_UI_task(){
    USB_Mainloop_Handler();
    cli();
    printf("Potentiomenter read %u%%\r\n", adc_read_percent());
    sei();
}

int event_polling_task(){
    static uint16_t lastPotReading = 0;
    uint16_t newPotReading;

    cli();
    // Get a new reading from potentiometer
    newPotReading = adc_read_percent();

    // Set flag if changed
    if(newPotReading != lastPotReading) {
        event_poll_flag = 1;
        lastPotReading = newPotReading;
    }
    sei();
    return 1;
}