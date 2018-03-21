/*
 * light.cpp
 *
 * Created: 3/20/2018 8:25:06 PM
 *  Author: Kovacs
 */ 

#include <avr/io.h>
#include "buffers.h"

extern outputBuffer outBuf;

void InitLight(void){
	outBuf.LowBeam = 0;
	outBuf.LeftFlasher = 0;
	outBuf.RightFlasher = 0;
}

void SOD_LeftFlasher(void){
	
}

void SOD_RightFlasher(void){
	
}

void SOD_LowBeam(void){
	
}