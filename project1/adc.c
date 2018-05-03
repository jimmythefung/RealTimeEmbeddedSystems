// initialize adc
void adc_init() {
    // AREF = AVcc
    ADMUX = (1<<REFS0);

    // ADC Enable and prescaler of 128; 16 MHz/128 = 125kHz
    ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    // Define analog signal input form ADC9 (pin 12, port D6)
    DIDR2 |= (1 << ADC9D);
    ADMUX |= (1 << MUX0);
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