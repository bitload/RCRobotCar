/*
 * motor.h
 *
 * Created: 3/20/2018 8:24:10 PM
 *  Author: Kovacs
 */ 


#ifndef MOTOR_H_
#define MOTOR_H_


void InitMotor(void);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_Motor12_0(uint8_t range);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_Motor12_1(uint8_t range);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_Motor12_2(uint8_t range);

/*
* range: 0 - 1
* default value: 0
*/
void SOD_Motor12_3(uint8_t range);

/*
* range: 0 - 100 (%)
* default value: 0
*/
void SODPWM_EnableMotor1(uint8_t range);

/*
* range: 0 - 100 (%)
* default value: 0
*/
void SODPWM_EnableMotor2(uint8_t range);



#endif /* MOTOR_H_ */