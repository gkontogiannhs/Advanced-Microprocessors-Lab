#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int turns = 4;

int main(void)
{	

	// PIN0, turn left
	PORTD.DIR |= PIN0_bm;
	PORTD.OUT |= PIN0_bm;
	
	// PIN2, represents straight line
	PORTD.DIR |= PIN2_bm;
	// turn on straight's led
	PORTD.OUTCLR = PIN2_bm;
	
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
		
	while(1){
		// walk straight
		// if perimeter completed, stop/ exit programm
		if (turns == 0)
			break;
	}
}


ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	// turn straight led off
	PORTD.OUT |= PIN2_bm;
	// turn on left led
	PORTD.OUTCLR = PIN0_bm;
	_delay_ms(2);
	// turn off left led
	PORTD.OUT |= PIN0_bm;
	// turn straight led on
	PORTD.OUTCLR = PIN2_bm;
	
	// reduce each side of the perimeter
	turns -= 1;
}



