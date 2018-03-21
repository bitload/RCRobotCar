/*
 * sensor.c
 *
 * Created: 3/20/2018 8:19:24 PM
 *  Author: Kovacs
 */ 

#include <avr/io.h>
#include "light_sensor.h"
#include "buffers.h"

extern inputBuffers inbuf;

void SIA_LightSensor(void){
	//read arduino d/a 0 pin
	inbuf.light_sensor = 980;
}

void InitLightSensor(void){
	inbuf.light_sensor = 0;
}
