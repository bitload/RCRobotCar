/*
 * light.h
 *
 * Created: 3/20/2018 8:25:27 PM
 *  Author: Kovacs
 */ 


#ifndef LIGHT_H_
#define LIGHT_H_

void InitLight(void);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_LeftFlasher(uint8_t value);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_RightFlasher(uint8_t value);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_LowBeam(uint8_t value);



#endif /* LIGHT_H_ */