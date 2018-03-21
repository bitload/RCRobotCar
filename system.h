#ifndef __SYSTEM
#define __SYSTEM

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "rtc_lib.h"

//--------Init Ports --------------------
void InitPorts(void);

//--------Init Timers --------------------
void InitTimers(void);

//--------Init Uart interface --------------------
void InitUart0(void);
void InitUart1(void);
void SendCharUart1(unsigned char ch);
unsigned char ReceiveCharUart1(void);
unsigned char ReceiveCharUart1_nonstop(void);
void deleteString(void);
void send_string0(const char s[]);
void send_string(const char s[]);
const char * getString0(void);
const char * getString(void);
void deleteString0(void);
void deleteString(void);
uint8_t get_esp_tx_ready(void);
void del_esp_tx_ready(void);
uint8_t get_computer_tx_ready(void);
void del_computer_tx_ready(void);
bool ButtonHandler(uint8_t pin);
static uint8_t menu_item;

void SetMenu1(void);
void SetMenu2(void);
void ExitMenu(void);

#define		B1				PA0
#define		B2				PA1
#define		B3				PA2
#define		B4				PA3
#define		B5				PA4

void CheckTemperatureAndUpdateRelay(void);

//---------Buzzer --------------------------------- 
void Buzzer(void);


//--------- Led On/Off ---------------------------------
void signalLedOn(void);
void signalLedOff(void);


typedef struct{
	struct thermostatSetValues{
		uint8_t phour;
		uint8_t pmin;
		float ptemp;
	};
	thermostatSetValues program[12];
}thermostatSetProgram;

typedef struct{
	thermostatSetProgram therm_day[7];
	unsigned long magic;
	uint8_t state;			//on auto = 1; on manual = 2; off = 3; * = 4
	float manual_temp;
	float idle_temp;
}thermostatSetDays;

void put_temp_prog_data(void);
bool enablePgmSet(thermostatSetDays days_mem, uint8_t nr_day, uint8_t nr_prog, uint8_t up, uint8_t itemnr);


#define		BTN_UP				((PINA&BIT0) == 0)
#define		BTN_LEFT			((PINA&BIT1) == 0)
#define		BTN_CENTER			((PINA&BIT2) == 0)
#define		BTN_RIGHT			((PINA&BIT3) == 0)
#define		BTN_DOWN			((PINA&BIT4) == 0)

#define 	RELAY_HIGH		PORTA |= BIT6
#define 	RELAY_LOW		PORTA &= ~BIT6

void createCustomChars(void);
void save_sms(const char * message);
void replaceAll(char * str, char oldChar, char newChar);


#endif


