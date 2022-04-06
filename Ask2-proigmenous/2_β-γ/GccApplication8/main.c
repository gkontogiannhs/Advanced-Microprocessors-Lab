#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int cnt = 0;
int still = 1;

int main(void)
{	

	// PIN0, turn left
	PORTD.DIR |= PIN0_bm;
	PORTD.OUT |= PIN0_bm;
	
	// PIN1, turn right
	PORTD.DIR |= PIN1_bm;
	PORTD.OUT |= PIN1_bm;
	
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
	
		// int
	PORTF.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc);
	
	sei();
	// start conversion
	ADC0.COMMAND |= ADC_STCONV_bm;
		
	while(still){
		// go straight
		PORTD.OUTCLR = PIN2_bm;
	}
}


ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	// increase counter
	cnt++;
	// if 1, 4, 5, 6, 7, turn left
	if(cnt == 1 || cnt == 4 || cnt == 5 || cnt == 6 || cnt == 7){
		turnLeft();
	}
	// if 2, 3 turn right
	else if(cnt == 2 || cnt == 3){
		turnRight();
	}
	
	// if perimeter completed
	if(cnt >= 7)
		still = 0;
	
}


void turnLeft(){
	
	// turn straight led off
	PORTD.OUT |= PIN2_bm;
	// turn on left led
	PORTD.OUTCLR = PIN0_bm;
	_delay_ms(2);
	// turn off left led
	PORTD.OUT |= PIN0_bm;
	// turn straight led on
	PORTD.OUTCLR = PIN2_bm;
}

void turnRight(){
	
	// turn straight led off
	PORTD.OUT |= PIN2_bm;
	// turn on right led
	PORTD.OUTCLR = PIN1_bm;
	_delay_ms(2);
	// turn off left led
	PORTD.OUT |= PIN1_bm;
	// turn straight led on
	PORTD.OUTCLR = PIN2_bm;
}


ISR(PORTF_PORT_vect) {
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	if(cnt == 1){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(1);
		turnRight();
		_delay_ms(2);
	}
	else if(cnt == 2){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnRight();
		_delay_ms(2)
	}
	else if(cnt == 3){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnRight();
		_delay_ms(2)
	}
	else if(cnt == 4){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(1);
		turnRight();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnRight();
		_delay_ms(2)
	}
	else if(cnt == 5){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(4);
		turnRight();
		_delay_ms(1);
		turnRight();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnRight();
		_delay_ms(2)
	}
	else if(cnt == 6){
		// turn straight led on
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(5);
		turnRight();
		_delay_ms(4);
		turnRight();
		_delay_ms(1);
		turnRight();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnLeft();
		_delay_ms(1);
		turnRight();
		_delay_ms(2)
	}
	
}
