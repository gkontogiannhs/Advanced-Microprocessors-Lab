#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
/*Using default clock 3.33MHz */

int stop_adc = 1;
int pulses = 0;
int first_time = 0;

void PORT_init(void);
void TCB0_init(void);
void ADC_init(void);

void TCB0_init (void)
{
	/* Load the Compare or Capture register with the timeout value*/
	TCB0.CCMP = 0x1020;
	TCB0.CNT = 0;
	/* Enable TCB3 and Divide CLK_PER by 2 */
	TCB0.CTRLA |= TCB_ENABLE_bm;
	TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	TCB0.INTCTRL |= TCB_CAPT_bm;

	/* Enable Pin Output and configure TCB in 8-bit PWM mode */
	TCB0.CTRLB |= TCB_CCMPEN_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
}

void TCB1_init (void)
{
		/* Load the Compare or Capture register with the timeout value*/
		TCB1.CCMP = 0x1020;
		TCB1.CNT = 0;
		/* Enable TCB3 and Divide CLK_PER by 2 */
		TCB1.CTRLA |= TCB_ENABLE_bm;
		TCB1.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
		TCB0.INTCTRL |= TCB_CAPT_bm;

		/* Enable Pin Output and configure TCB in 8-bit PWM mode */
		TCB1.CTRLB |= TCB_CCMPEN_bm;
		TCB1.CTRLB |= TCB_CNTMODE_PWM8_gc;
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
	while(stop_adc & first_time){
		
	}
}



int main(void)
{
	
	// set outputs
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
	TCB0_init();
	TCB1_init();
	
	sei();
	
	while(1){
		;
	}
}


ISR(TCB0_INT_vect)
{
		/* if even, turn on */
		if (pulses % 2 == 0){
			PORTD.OUTCLR = PIN0_bm;
			/* If second timer also completed */
			if (TCB1.INTFLAGS == 1)
				PORTD.OUTCLR = PIN1_bm;
			pulses++;
		/* if even, turn on */
		}else{
			PORTD.OUT |= PIN0_bm;
			/* If second timer also completed */
			if (TCB1.INTFLAGS == 1)
				PORTD.OUT |= PIN1_bm;
			pulses++;
		}	
		
	TCB0.INTFLAGS = TCB_CAPT_bm; /* Clear the interrupt flag */
	PORTB.IN = PIN5_bm; /* Toggle PB5 GPIO */
}

ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;

	/* enable LED and wait for Right or Left command */
	PORTD.OUTCLR = PIN2_bm;
	_delay_ms(1);

	first_time = 1;
	stop_adc = 0;
}


ISR(PORTF_PORT_vect){
	/* Get flags to check for left or right */
	int intflags = PORTF.INTFLAGS; 
	
	/* If switch5 */
	int temp = intflags &= ~(00100000);
	if (temp == 1){
		/* Turn right --> double the period of left wheel*/
		TCB1.CCMP = 0x2040;
	}
	/* If switch6 */
	else{
		/* Turn left --> double the period of right wheel*/
		TCB0.CCMP = 0x2040;
	}
	
	stop_adc = 0;
	
	/* TURN OFF DECISION LED */
	PORTD.OUT |= PIN2_bm;
}

//