#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int pass_counter = 0;
int pass_input = 0;
int timer_after_pass = 0;
int tries = 0;
int is_robbed = 0;
int is_robbed_timer = 0;
int pulses = 0;

int main(void){

	// set switches
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	while(1){
		enable_alarm();
		
		ADC_init();
		
		disable_alarm();
	}
}

void enable_alarm()
{
	sei();
	while(!pass_input){
		
	}
	//pass_input = 0;
	//Timer set up
	TCA0.SINGLE.CNT = 0; //Clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode
	TCA0.SINGLE.CMP0 = 1; //Stop turning when this value is reached
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.CTRLA |= 1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
	cli();
	
	// start conversion
	ADC0.COMMAND |= ADC_STCONV_bm;
	sei();
	while(!timer_after_pass){
		// waiting for timer
	}
	cli();
}

void ADC_init(void)
{
	
	// led 0 is output
	PORTD.DIR |= PIN0_bm;
	
	// initialize ADC
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;
	ADC0.CTRLA |= ADC_FREERUN_bm;
	ADC0.CTRLA |= ADC_ENABLE_bm;
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;
	
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	
	// limit is 5
	ADC0.WINLT |= 10;
	ADC0.INTCTRL |= ADC_WCMP_bm;
	ADC0.CTRLE |= ADC_WINCM0_bm;
	
	sei();
	// START CONVERSION
	ADC0.COMMAND |= ADC_STCONV_bm;
	// wait till find target
	while(!is_robbed){
		
	}
	cli();
	
}

void disable_alarm(){
	//Timer set up
	TCA0.SINGLE.CNT = 0; //Clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode
	TCA0.SINGLE.CMP0 = 10; //Stop turning when this value is reached
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.CTRLA |= 1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
	cli();
	
	// start conversion
	ADC0.COMMAND |= ADC_STCONV_bm;
	sei();
	while(!is_robbed_timer){
		// waiting for timer
	}
	cli();
}

void siren(){
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


ISR(PORTF_PORT_vect){
	/* Get flags to check for left or right */
	int intflags = PORTF.INTFLAGS;
	
	/* If switch5 */
	int sw5 = intflags &= ~(00100000);
	int sw6 = intflags &= ~(01000000);
	
	if(sw6 == 64 && pass_counter == 0){
		pass_counter++;
	}
	else if(sw5 == 32 && pass_counter == 1){
		pass_counter++;
	}
	else if(sw5 == 32 && pass_counter == 2){
		pass_counter++;
	}
	else if(sw6 ==64 && pass_counter == 3){
		pass_counter++;
	}
	else{
		pass_counter = 0;
		tries++;
	}
	
	if (tries == 3){
		siren();
	}
	else if(pass_counter == 4){
		pass_input = 1;
		is_robbed_timer = 1;
	}
}

ISR(TCA0_CMP0_vect){
	//Clear timer interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	if(pass_input == 1){
		//Disable Timer
		TCA0.SINGLE.CTRLA = 0;
		// turn on flag
		timer_after_pass = 1;
		// clear correct password flag
		pass_input = 0;
		
		}else if(pass_input == 0){
		siren();
	}
}


ISR(ADC0_WCOMP_vect){
	// clear flags
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;

	/* enable LED and wait for Right or Left command */
	is_robbed = 1;
	is_robbed_timer = 0;
	pass_counter = 0;
	// turn led0 on
	_delay_ms(1);
	PORTD.OUTCLR = PIN0_bm;
}



ISR(TCB0_INT_vect)
{
	if (pass_counter != 4){
		/* if even, turn on */
		if (pulses % 2 == 0)
		PORTD.OUTCLR = PIN0_bm;
		else
		PORTD.OUT |= PIN0_bm;

		pulses++;
		
	}
	else{
		// disable pwm
		TCB0.CTRLA = 0;
	}
	TCB0.INTFLAGS = TCB_CAPT_bm; /* Clear the interrupt flag */
	PORTB.IN = PIN5_bm; /* Toggle PB5 GPIO */
}
