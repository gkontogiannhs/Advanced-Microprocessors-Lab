#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

#define del_min 1
#define del_max 5

#define ped 30

int interr_f = 0;
int interr_p = 0;

int main(void)
{
    int rn = 0;
	int flag = 0;
	
	// PIN0 --> pedestrians --> initially off
	PORTD.DIR |= PIN0_bm;
	PORTD.OUT |= PIN0_bm;
	
	// PIN1 --> small road --> initially off
	PORTD.DIR |= PIN1_bm;
	PORTD.OUT |= PIN1_bm;
	
	// PIN2 --> big road --> initially on
	PORTD.DIR |= PIN2_bm;
	PORTD.OUTCLR = PIN2_bm;
	
	//enable flag interrupts
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; 
	
	// initialize timer
	TCA0.SINGLE.CNT = 0;
	TCA0.SINGLE.CTRLB = 0;
	// time for pedestrians
	TCA0.SINGLE.CMP0 = ped;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;

	while(1) 
    {
		// check for flag interr
		sei();
		// while no pedestrians
		while(!interr_f)
		{
			rn = rand() % 10;
			// if car traced 
			if(rn == 0 || rn == 5 || rn == 8)
			{
				
				// set flag
				flag = 1;
			
				// turn red big road
				PORTD.OUT |= PIN2_bm;
			
				// wait some time before small road turns green
				_delay_ms(del_min);
			
				// turn green small road
				PORTD.OUTCLR = PIN1_bm;	 
			}
			
			// turn it back green if it was set red
			if(flag)
			{
				PORTD.OUT |= PIN1_bm;
				PORTD.OUTCLR = PIN2_bm;
			}
		}
		
		// big road, turn red
		PORTD.OUT |= PIN2_bm;
		
		// enable timer
		TCA0.SINGLE.CTRLA |= 1;
		TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
		
		// green for pedestrians
		PORTD.OUTCLR = PIN0_bm;
		
		//green for small road
		PORTD.OUTCLR = PIN1_bm;
		
		sei();
		while(!interr_p){
			// wait
		}
		cli();
		
		// zero flag timer
		interr_p = 0;
		// red for pedestrians
		PORTD.OUT |= PIN0_bm;
		
		// zero pedestrian flag
		interr_f = 0;
		
		// red for small road
		PORTD.OUT |= PIN1_bm;
		
		// turn back red for big
		PORTD.OUTCLR = PIN2_bm;
		
		// empty counter
		TCA0.SINGLE.CNT = 0;
    }
	cli();
}


// flag interrupt
ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	interr_f = 1;
}


// timer interrupt
ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0;
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	interr_p = 1;
}
