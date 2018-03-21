#include "delay.h"

// very simple delay
// CPU oscilator is 16MHz
// 1 instr = 0.0625 uS
// 1 cycle = 4 instruction					
// 1 us = 4 cycle

void delay_ms(uint16_t time){
	uint16_t i;
	for (i=1;i<=time;i++){
		_delay_ms(1);
	}
}
void delay_us(uint16_t time){
	uint16_t i;
	for (i=1;i<=time;i++){
		_delay_us(1);
	}
}
