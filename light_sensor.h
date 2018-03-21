/*
 * sensor.h
 *
 * Created: 3/20/2018 8:19:45 PM
 *  Author: Kovacs
 */ 


#ifndef SENSOR_H_
#define SENSOR_H_


#include "light_sensor.h"

void InitLightSensor(void);

/*
* range: 0 - 1023
* default value: 1023
*/
void SIA_LightSensor(void);


#endif /* SENSOR_H_ */