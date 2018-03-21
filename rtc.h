#ifndef __RTC
#define __RTC

void InitRTC(void);


struct time{
	unsigned char second;																																														
	unsigned char minute;																			
	unsigned char hour;
	unsigned char date;
	unsigned char month;																																					
	uint16_t year;
	uint8_t wait;
};				

extern uint8_t sec_refresh;

#endif