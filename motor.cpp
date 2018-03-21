/*
 * motor.cpp
 *
 * Created: 3/20/2018 8:23:48 PM
 *  Author: Kovacs
 */ 

#include <avr/io.h>
#include "motor.h"
#include "buffers.h"

outputBuffer outBuf;

void InitMotor(void){
	outBuf.EnableMotor1 = 0;
	outBuf.EnableMotor2 = 0;
	outBuf.LeftFlasher = 0;
	outBuf.LowBeam = 0;
	outBuf.Motor12_0 = 0;
	outBuf.Motor12_1 = 0;
	outBuf.Motor12_2 = 0;
	outBuf.Motor12_3 = 0;
	outBuf.RightFlasher = 0;
}

void SOD_Motor12_0(void){

}

void SOD_Motor12_1(void){
	
}

void SOD_Motor12_2(void){
	
}

void SOD_Motor12_3(void){
		
}

void SODPWM_EnableMotor1(void){
	
}

void SODPWM_EnableMotor2(void){
	
}

