#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*Using default clock 3.33MHz */

int PERIOD_VALUE_L = 128;
int PERIOD_VALUE_H = 128;
int DUTY_CYCLE_VALUE_L = 64;
int DUTY_CYCLE_VALUE_H = 64;

int stop_adc = 1;

void TCA0_init(void);
void PIN_init(void);
void TCA0_hardReset(void);

void TCA0_init(void)
{
    /* set waveform output on PORT A */
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;
    
    /* enable split mode */
    TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;                 
    
    TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm        /* enable compare channel 0 for the higher byte */
                     | TCA_SPLIT_LCMP0EN_bm;    /* enable compare channel 0 for the lower byte */
					 
	// by default 128 resolution
	TCA0_set_periods(128, 128, 64, 64);
    
    TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc    /* set clock source (sys_clk/16) */
                     | TCA_SPLIT_ENABLE_bm;         /* start timer */
}

void ADC_init(void){
	
		// initialize ADC
		ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;
		ADC0.CTRLA |= ADC_FREERUN_bm;
		ADC0.CTRLA |= ADC_ENABLE_bm;
		ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;
		
		ADC0.DBGCTRL |= ADC_DBGRUN_bm;
		
		// limit is 5
		ADC0.WINLT |= 5;
		ADC0.INTCTRL |= ADC_WCMP_bm;
		ADC0.CTRLE |= ADC_WINCM0_bm;
		
		sei();
		// start conversion
		ADC0.COMMAND |= ADC_STCONV_bm;
		while(stop_adc){
			
		}
}

/* must be used when switching from single mode to split mode */
void TCA0_hardReset(void)
{
    /* stop timer */
    TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);  
    
    /* force a hard reset */
    TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc; 
}

int main(void)
{
	
    PORTD.DIR |= PIN0_bm    /* set pin 0 of PORT A as output */
              | PIN1_bm    /* set pin 1 of PORT A as output */
			  | PIN2_bm;
    // all off			  
	PORTD.OUT |= PIN0_bm;
	PORTD.OUT |= PIN1_bm;
	PORTD.OUT |= PIN2_bm;
	
	// set switches
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	ADC_init();
    TCA0_init();
	
	sei();
	
    while(1){
        
    }
}


void TCA0_set_periods(int L_period, int H_period, int L_duty, int H_duty){
	/* set the PWM frequencies and duty cycles */
	TCA0.SPLIT.LPER = L_period;
	TCA0.SPLIT.LCMP0 = H_period;
		
	TCA0.SPLIT.HPER = L_duty;
	TCA0.SPLIT.HCMP0 = H_duty;
}


ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
	// enable LED and wait command
	PORTD.OUTCLR = PIN2_bm;
	_delay_ms(1);
	// TCA0.SPLIT.CTRLB = 0;
	stop_adc = 0;
}

ISR(TCA0_OVF_vect){
	// clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUTCLR = PIN0_bm;
	PORTD.OUTCLR = PIN1_bm;
}

ISR(TCA0_CMP0_vect){
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUT |= PIN0_bm;
	PORTD.OUT |= PIN1_bm;
}


ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	// TURN OFF DECISION LED
	PORTD.OUT |= PIN2_bm;
	if (intflags &= ~(00100000)){
		TCA0_set_periods(64, 128, 32, 64);
	}
	else if (intflags &= ~(01000000)){
		TCA0_set_periods(128, 64, 64, 32);
	}
	stop_adc = 0;
}

//