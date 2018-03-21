/*
 * buffers.h
 *
 * Created: 3/20/2018 10:35:23 PM
 *  Author: Kovacs
 */ 


#ifndef BUFFERS_H_
#define BUFFERS_H_


typedef struct inputBuffer_s{
	 uint16_t light_sensor;
	 uint8_t WifiControlUp;
	 uint8_t WifiControlDown;
	 uint8_t WifiControlRight;
	 uint8_t WifiControlLeft; 
}inputBuffers;
 
typedef struct outputBuffer_s{	 
	 uint8_t Motor12_0;
	 uint8_t Motor12_1;
	 uint8_t Motor12_2;
	 uint8_t Motor12_3;
	 uint8_t EnableMotor1;
	 uint8_t EnableMotor2;
	 uint8_t LeftFlasher;
	 uint8_t RightFlasher;
	 uint8_t LowBeam;	 
}outputBuffer;



#endif /* BUFFERS_H_ */