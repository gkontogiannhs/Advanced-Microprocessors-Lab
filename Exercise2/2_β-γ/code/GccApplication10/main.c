#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define t 10; // 1 second delay

int cnt = 0;
int still = 1;
int t_interr = 0;
int result = 0;
int reverse = 0;

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
	ADC0.WINHT |= 20;
	ADC0.INTCTRL |= ADC_WCMP_bm;
	ADC0.CTRLE |= ADC_WINCM0_bm;
	
	// switch interrupt enable
	PORTF.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc);
	
	//Timer set up
	TCA0.SINGLE.CNT = 0; //Clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode
	TCA0.SINGLE.CMP0 = t; //Stop turning when this value is reached
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
	
	while(1){
		// go straight
		sei();
		// start conversion
		ADC0.COMMAND |= ADC_STCONV_bm;
		
		PORTD.OUTCLR = PIN2_bm;
		_delay_ms(1);
		// check for right distance
		single();
		
		// perimeter is completed. Stop device
		if(!still)
		break;
	}
}


ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
	if(!reverse){
		if (result <=5){
			cnt++;
			turnLeft();

			}else if(result >= 20){
			cnt++;
			turnRight();
		}
		// reverse mode enabled
		}else{
		// increase counter
		if (result <=5){
			cnt--;
			turnRight();

			}else if(result >= 20){
			cnt--;
			turnLeft();
		}
	}
	// CLEAR RES
	ADC0.RES = 0;
	// if perimeter completed
	if(cnt >= 7 || cnt <= 0)
	still = 0;
	
}

ISR(TCA0_CMP0_vect){
	//TCA0.SINGLE.CTRLA = 0; //Disable Timer
	TCA0.SINGLE.CTRLA &= 0xFE;
	//Clear timer interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	t_interr = 1;
}


ISR(PORTF_PORT_vect) {
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	reverse = 1;
}




void turnLeft(){
	
	// turn straight led off
	PORTD.OUT |= PIN2_bm;
	// turn on left led
	PORTD.OUTCLR = PIN0_bm;
	_delay_ms(2);
	// enable timer
	/*
	TCA0.SINGLE.CTRLA |= 1;
	sei();
	while(!t_interr){
		// wait timer
	}
	//TCA0.SINGLE.CTRLA &= 0xFE;
	cli();
	*/
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


void single(){
	// clear ADC
	ADC0.CTRLA = 0;
	// initialize ADC
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;
	ADC0.CTRLA |= ADC_ENABLE_bm;
	
	ADC0.CTRLE = 0x02;
	sei();
	// start conversion
	ADC0.COMMAND |= ADC_STCONV_bm;
	result = ADC0.RES;
	_delay_ms(2);
	
	// re-set ADC to free-running
	ADC0.CTRLA |= ADC_FREERUN_bm;
	ADC0.CTRLE = 0x01;
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	// start conversion
	ADC0.COMMAND |= ADC_STCONV_bm;
}
