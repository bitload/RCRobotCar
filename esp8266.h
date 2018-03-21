/*
 * esp8266.h
 *
 * Created: 3/20/2018 8:30:04 PM
 *  Author: Kovacs
 */ 


#ifndef ESP8266_H_
#define ESP8266_H_

void InitESP8266(void);

/*
* range: 0 - 1
* default value: 0
*/
void SID_WifiControlUp(void);

/*
* range: 0 - 1
* default value: 0
*/
void SID_WifiControlDown(void);

/*
* range: 0 - 1
* default value: 0
*/
void SID_WifiControlRight(void);

/*
* range: 0 - 1
* default value: 0
*/
void SID_WifiControlLeft(void);



#endif /* ESP8266_H_ */