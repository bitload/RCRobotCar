/*
 * RCRobotCar.cpp
 *
 * Created: 3/20/2018 10:06:50 PM
 * Author : Kovacs
 */ 

#include <avr/io.h>
#include "light_sensor.h"
#include "esp8266.h"
#include "motor.h"
#include "light.h"


int main(void)
{
    /* Replace with your application code */
		
	InitMotor();
	InitLight();
	InitLightSensor();
	InitESP8266();
	
    while (1) 
    {
    }
}

