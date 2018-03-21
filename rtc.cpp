/*
 * rtcext.c
 *
 * Created: 3/11/2017 5:58:51 PM
 *  Author: Endre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "rtc.h"
#include "lcd.h"
#include "system.h"


/*
FUSES =
{
	.low = 0x3F,		//Select cpu clk: Internal calibrated RC-oscillator @ 4MHz
	.high = 0x99,		//Default settings
	.extended = 0xE3,	//Default settings
};
*/

time rtcext;
uint8_t sec_refresh;

void InitRTC(void)
{
	//Wait for external clock crystal to stabilize;
	for (uint8_t i=0; i<0x40; i++)
	{
		for (uint16_t j=0; j<0xFFFF; j++);
	}
	//DDRB = 0xFF;											//Configure all eight pins of port B as outputs
	TIMSK &= ~((1<<TOIE0)|(1<<OCIE0));						//Make sure all TC0 interrupts are disabled
	ASSR |= (1<<AS0);										//set Timer/counter0 to be asynchronous from the CPU clock
	//with a second external clock (32,768kHz)driving it.
	TCNT0 =0;												//Reset timer
	TCCR0 =(1<<CS00)|(1<<CS02);								//Prescale the timer to be clock source/128 to make it
	//exactly 1 second for every overflow to occur
	while (ASSR & ((1<<TCN0UB)|(1<<OCR0UB)|(1<<TCR0UB)))	//Wait until TC0 is updated
	{}
	TIMSK |= (1<<TOIE0);									//Set 8-bit Timer/Counter0 Overflow Interrupt Enable
	sei();													//Set the Global Interrupt Enable Bit
	//set_sleep_mode(SLEEP_MODE_PWR_SAVE);					//Selecting power save mode as the sleep mode to be used
	//sleep_enable();											//Enabling sleep mode
	sec_refresh = 0;
	
}


ISR(TIMER0_OVF_vect)
{		
	sec_refresh = 1;
}
