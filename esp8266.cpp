/*
 * esp8266.cpp
 *
 * Created: 3/20/2018 8:03:48 PM
 *  Author: Kovacs
 */ 

#include <avr/io.h>
#include "esp8266.h"
#include "buffers.h"

inputBuffers inbuf;

void InitESP8266(void){
	inbuf.WifiControlDown = 0;
	inbuf.WifiControlLeft = 0;
	inbuf.WifiControlRight = 0;
	inbuf.WifiControlUp = 0;
}

void SID_WifiControlUp(void){
	
	inbuf.WifiControlUp = 0;
}

void SID_WifiControlDown(void){
	inbuf.WifiControlDown = 0;
}

void SID_WifiControlRight(void){
	inbuf.WifiControlRight = 0;
}

void SID_WifiControlLeft(void){
	inbuf.WifiControlLeft = 0;
}