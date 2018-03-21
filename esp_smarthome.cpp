/*********************************************
Project : ATmega128
Version : 01
Date    : 2017.02.27
Author  : Kovacs Endre                            
Comments: ESP8266 Communication


Chip type           : ATmega128
Program type        : Application
Clock frequency     : 14.7456 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 1024
*********************************************/
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "system.h"
#include "bits.h"
#include "lcd.h"
#include "at_commands.h"
#include "ds18x20lib.h"
#include "Menu.h"
#include "rtc.h"
#include "twi.h"
#include "rtc_lib.h"
#include "Sim800l.h"
#include "eeprom.h"
                                                        
extern Sim800l Sim8;

/*
#define		B1				(PINA&BIT0)
#define		B2				(PINA&BIT1)
#define		B3				(PINA&BIT2)
#define		B4				(PINA&BIT3)
#define		B5				(PINA&BIT4)
*/

#define		TXD				(PIND&BIT3)	
#define		RXD				(PIND&BIT2)	
#define		DALLAS			(PINA&BIT5)

#define clampValue(val, lo, hi) if (val > hi) val = hi; if (val < lo) val = lo;
#define maxValue(a, b) ((a > b) ? a : b)
#define minValue(a, b) ((a < b) ? a : b)

Menu::Engine *engine;
     
char * esp_response;
const char OKrn[] = "OK\r\n";
char * rec_ok;

#define LCD_CHARS 20
#define LCD_LINES  4
extern struct time rtcext;
bool disable_button;
bool editThrm(const Menu::Action_t a);

extern thermostatSetDays EEMEM EEthrmSetDays;	
thermostatSetDays thrmSetDaysMain = {};
extern uint8_t enable_update_relay;
extern uint8_t display_temp_refresh_ct;
extern uint8_t check_sms_and_update_busy;
uint8_t readSMS;
uint16_t eeprom_address_read;
uint16_t last_sms_addr;

typedef enum {
	None		= 0,
	Idle		= 1,
	Edit		= 2,
	EditTime	= 3,
	EditDate	= 4,
	EditThrmMon = 5,
	EditThrmTue = 6,
	EditThrmWed = 7,
	EditThrmThu = 8,
	EditThrmFri = 9,
	EditThrmSat = 10,
	EditThrmSun = 11,
	EditThermostatState	= 12,
	EditDeleteSMS = 13,
	EditReadSMS = 14,
} State;

State systemState = Idle;
State previousSystemState = None;
uint8_t setItemNr = 1;
uint8_t prevSetItemNr = 0;
extern const Menu::Item_t miEditThermMon, miEditThermTue, miEditThermWed, miEditThermThu, miEditThermFri, miEditThermSat, miEditThermSun;

// ----------------------------------------------------------------------------

bool menuExit(const Menu::Action_t a) {	
	systemState = Idle;
	return true;
}

bool menuDummy(const Menu::Action_t a) {
	return true;
}

bool editTime(const Menu::Action_t a) {
	systemState = EditTime;
	return true;
}

bool editDate(const Menu::Action_t a) {
	systemState = EditDate;
	return true;
}


bool editThrm(const Menu::Action_t a) {
	if (engine->currentItem == &miEditThermMon) systemState = EditThrmMon;
	if (engine->currentItem == &miEditThermTue) systemState = EditThrmTue;
	if (engine->currentItem == &miEditThermWed) systemState = EditThrmWed;
	if (engine->currentItem == &miEditThermThu) systemState = EditThrmThu;
	if (engine->currentItem == &miEditThermFri) systemState = EditThrmFri;
	if (engine->currentItem == &miEditThermSat) systemState = EditThrmSat;
	if (engine->currentItem == &miEditThermSun) systemState = EditThrmSun;
	return true;
}

bool editThermostatState(const Menu::Action_t a) {
	systemState = EditThermostatState;
	return true;
}

bool editDeleteSMS(const Menu::Action_t a) {
	systemState = EditDeleteSMS;
	return true;
}

bool editReadSMS(const Menu::Action_t a) {
	systemState = EditReadSMS;
	return true;
}

bool menuBack(const Menu::Action_t a) {
	if (a == Menu::actionDisplay) {
		engine->navigate(engine->getParent(engine->getParent()));
	}
	return true;
}
// ------------------------------------------------------------------------------------------------------------------
//							Name, Label, Next, Previous, Parent, Child, Callback

MenuItem(miExit, "", Menu::NullItem, Menu::NullItem, Menu::NullItem, miEditTimeDate, menuExit);

MenuItem(miEditTimeDate, "Time/Date", miEditThermLabel, miExit, miExit, miEditTimeLabel, menuDummy);
	MenuItem(miEditTimeLabel, "Time", miEditDateLabel,    miEditTimeDate,    miEditTimeDate, miEditTime, menuDummy);
		MenuItem(miEditTime, "", Menu::NullItem,    miEditTimeLabel,    miEditTimeLabel, Menu::NullItem, editTime);
		
	MenuItem(miEditDateLabel, "Date", Menu::NullItem,    miEditTimeLabel,    miEditTimeDate, miEditDate, menuDummy);
		MenuItem(miEditDate, "", Menu::NullItem,    miEditDateLabel,    miEditDateLabel, Menu::NullItem, editDate);
		
MenuItem(miEditThermLabel, "Thermostat", miEditStateLabel,   miEditTimeDate,  miExit,	miEditThermLabelMon, menuDummy);
	MenuItem(miEditThermLabelMon, "Mon", miEditThermLabelTue,    miEditThermLabel,		miEditThermLabel, miEditThermMon, menuDummy);
	MenuItem(miEditThermMon, "", Menu::NullItem,    miEditThermLabelMon,    miEditThermLabelMon, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelTue, "Tue", miEditThermLabelWed,		miEditThermLabelMon,    miEditThermLabel, miEditThermTue, menuDummy);
	MenuItem(miEditThermTue, "", Menu::NullItem,    miEditThermLabelTue,    miEditThermLabelTue, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelWed, "Wed", miEditThermLabelThu,    miEditThermLabelTue,    miEditThermLabel, miEditThermWed, menuDummy);
	MenuItem(miEditThermWed, "", Menu::NullItem,    miEditThermLabelWed,    miEditThermLabelWed, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelThu, "Thu", miEditThermLabelFri,		miEditThermLabelWed,    miEditThermLabel, miEditThermThu, menuDummy);
	MenuItem(miEditThermThu, "", Menu::NullItem,    miEditThermLabelThu,    miEditThermLabelThu, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelFri, "Fri",   miEditThermLabelSat,		miEditThermLabelThu,    miEditThermLabel, miEditThermFri, menuDummy);
	MenuItem(miEditThermFri, "", Menu::NullItem,    miEditThermLabelFri,    miEditThermLabelFri, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelSat, "Sat", miEditThermLabelSun,		miEditThermLabelFri,    miEditThermLabel, miEditThermSat, menuDummy);
	MenuItem(miEditThermSat, "", Menu::NullItem,    miEditThermLabelSat,    miEditThermLabelSat, Menu::NullItem, editThrm);

	MenuItem(miEditThermLabelSun, "Sun",   Menu::NullItem,			miEditThermLabelSat,	 miEditThermLabel, miEditThermSun, menuDummy);
	MenuItem(miEditThermSun, "", Menu::NullItem,    miEditThermLabelSun,    miEditThermLabelSun,	Menu::NullItem, editThrm);	
	
MenuItem(miEditStateLabel, "State", miEditDeleteSMSLabel,   miEditThermLabel,  miExit,	miEditState, menuDummy);
	MenuItem(miEditState, "", Menu::NullItem,    Menu::NullItem,    miEditStateLabel, Menu::NullItem, editThermostatState);
	
MenuItem(miEditDeleteSMSLabel, "Delete SMS", miReadSMSLabel,   miEditStateLabel,  miExit,	miEditDeleteSMS, menuDummy);
	MenuItem(miEditDeleteSMS, "", Menu::NullItem,    Menu::NullItem,    miEditDeleteSMSLabel, Menu::NullItem, editDeleteSMS);
	
MenuItem(miReadSMSLabel, "Read Last SMS", Menu::NullItem,   miEditDeleteSMSLabel,  miExit,	miReadSMS, menuDummy);
	MenuItem(miReadSMS, "", Menu::NullItem,    Menu::NullItem,    miReadSMSLabel, Menu::NullItem, editReadSMS);		

// ----------------------------------------------------------------------------


void SetupMenu() {
	engine = new Menu::Engine(&Menu::NullItem);
	menuExit(Menu::actionNone); // reset to initial state
}

void my_select(void *arg, char *name);



void renderMenuItem(const Menu::Item_t *mi, uint8_t pos) {
	lcd_gotoxy(0, pos);

	// cursor
	if (engine->currentItem == mi) {
		lcd_data((uint8_t)IconBlock);
	}
	else {
		lcd_putc(20); // space
	}

	lcd_puts(engine->getLabel(mi));

	// mark items that have children
	if (engine->getChild(mi) != &Menu::NullItem) {
		lcd_putc(20); // space
		lcd_data((uint8_t)IconRight);
		lcd_puts("      ");		// ha eltünik a scroll karakter vagy az elöz? sor eleje akkor csökkenteni kell!!!
	}
}

// ----------------------------------------------------------------------------

const char * getLabelSetTimeHourMinute(const uint8_t setItem){
	switch(setItem){	
		case 1: return "Hour";
		case 2: return "Minute";
	}
	return "Hour";
}

const char * getLabelSetTimeDate(const uint8_t setItem){
	switch(setItem){
		case 1: return "Year";
		case 2: return "Month";
		case 3: return "Day";
		case 4: return "Weekday";
	}
	return "Year";
}

const char * getLabelSetThrmDay(const uint8_t setItem){
	switch(setItem){
		case EditThrmMon: return "Mon";
		case EditThrmTue: return "Tue";
		case EditThrmWed: return "Wed";
		case EditThrmThu: return "Thu";
		case EditThrmFri: return "Fri";
		case EditThrmSat: return "Sat";
		case EditThrmSun: return "Sun";
	}
	return "Mon";
}

// ----------------------------------------------------------------------------

bool ReadButton(uint8_t port, uint8_t pin){

	if (!(port & (1<<pin))){
		return true;
	}
	else return false;
}

bool ButtonHandler(uint8_t pin){
	bool menu_event = false;
	static bool button_pressed;
	bool button_now_pressed = ReadButton(PINA, pin);
	menu_event = button_now_pressed && !button_pressed;
	button_pressed = button_now_pressed;
	return menu_event;
}

unsigned long magic_number;

uint8_t index;   // to indicate the message to read.

int main() {     


	
	/*
	delay_ms(1000);
	signalLedOff();
	delay_ms(1000);
	signalLedOn();
	delay_ms(1000);
	signalLedOff();
	delay_ms(1000);	
	*/

	//Ports 	
	InitPorts();		
	
	/* initialize display, cursor off */
	lcd_init(LCD_DISP_ON);	
	
	/* Build custom characters*/
	createCustomChars();	

	//Init Uart0
	InitUart0();
	
	//Init Uart1 
	InitUart1();
	
	send_string("\nStarting...\n");	

	//RTC Init
	InitRTC();		

	//SIM800L Init
	Sim8.InitSIM();
	
	SetupMenu();	               	 

	twi_init_master();
		
	rtc_init();
	
	uint8_t menuItemsVisible = LCD_LINES;
	
/*		
	char * access_point = "JUJI";
	char * password = "zoldalma";
		

	SEND_CMD(CLR_DISP);
	SEND_CMD(DD_RAM_ADDR0);
	SEND_STR("Starting...");
	
	delay_ms(5000);											// wait for esp init
		
	send_command_to_esp(AT_CWJAP_Q, 2000);					// connected?
	
	message_in = getString0();
	send_string(message_in);	
		
	if (strstr(message_in, access_point) == 0){

		send_command_to_esp(AT_RST, 1000);
		SEND_CMD(DD_RAM_ADDR0);
		SEND_STR("Reset...");
		
		send_command_to_esp(AT_CWJAP, 10);
		send_command_to_esp(access_point, 10);
		send_command_to_esp(password, 10);
		send_command_to_esp("\r\n", 5000);
		message_in = getString0();
		send_string(message_in);
		if (strstr(message_in, OKrn) == 0){
			SEND_CMD(DD_RAM_ADDR0);
			SEND_STR("Error. Not Connected.");
			while(1){}
		}
	}	
	
	send_command_to_esp(AT_CIPMUX, 2000);
	send_command_to_esp(AT_CIPSERVER, 2000);	
*/
	ds1820_init(DS1820_pin_sens);
/*
	int i = 0;
	while(i<=200){
		Buzzer();
		i++;
	}	
	*/
	ds1820_init(DS1820_pin_sens);
	
	static char sbuffer[21];
	menu_item = 0;
	uint8_t updateMenu = 0;
	float ftemp;
	unsigned int displ_count = 0;
	static char rtcBuffer[20];		
	static char buttonDown;
		


	extern struct tm* rtctime;
	//rtc_set_time_s(12, 0, 50);
	/*
	rtctime->sec = 0;
	rtctime->min = 0;
	rtctime->hour = 0;
	rtctime->mday = 1;
	//rtctime->wday = 7;	
	rtctime->mon = 1;
	rtctime->year = 17;
	rtc_set_time(rtctime);
	*/
	uint8_t thProgram = 1;
	
	//thermostatSetDays EEthrmSetDays = get_EEthrmSetDays();
	
	eeprom_read_block(&thrmSetDaysMain, &EEthrmSetDays, sizeof(thrmSetDaysMain));
	magic_number = 1235;
	
	if (thrmSetDaysMain.magic != magic_number){

		thrmSetDaysMain = {{
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
			{{{0,0,20.0},{4,0,20.0},{8,0,20.0},{12,0,20.0},{16,0,20.0},{20,0,20.0}}},
		}, .magic = magic_number};
		thrmSetDaysMain.manual_temp = 16.0;		
		thrmSetDaysMain.idle_temp = 5.0;
		eeprom_write_block(&thrmSetDaysMain, &EEthrmSetDays, sizeof(thrmSetDaysMain));
	}	
	
	thrmSetDaysMain.state = 1;			
	
	uint32_t button_scan_cycle = 0;	
	uint8_t wait_for_send = 0;
	
	deleteString();	
	deleteString0();
				
	send_string("Started\n");			
		
	readSMS = 0;
	
	set_last_txt_address(0);
	eeprom_address_read = 0;
	
	while (1) {  
			
			
		if (sec_refresh == 1){			
			// 1s. frissites

			CheckTemperatureAndUpdateRelay();						
				
			int nr_sms_messages;
			nr_sms_messages = strtol(Sim8.GetNrOfSms(), NULL, 10);
				
			if (nr_sms_messages > 35){
				if(wait_for_send > 40){			//40s-ot var az utan hogy bejott egy sms mert amig nem kuld valaszt addig nem tudja torolni az smseket
					Sim8.DeleteAllSMS();
					Sim8.ResetNrOfSms();
					send_string("ALL SMS DELETED\n");
					wait_for_send = 0;
				}
				wait_for_send++;
			}							
					
		}
			
//****************************************************** ESP8266 handler ******************************************************
		/*
		if (get_esp_tx_ready() == 1){
			message_in = getString0();
			send_string(message_in);
			
			if (strstr(message_in, "RELAY ON") != 0){
				RELAY_HIGH;
			}	
			
			if (strstr(message_in, "RELAY OFF") != 0){
				RELAY_LOW;
			}
			if (strstr(message_in, "TEMP?") != 0){
				send_command_to_esp("AT+CIPSEND=0,16\r\n", 1000);				
			}	
			if (strstr(message_in, ">") != 0){
				SEND_CMD(DD_RAM_ADDR0);
				SEND_STR((unsigned char*)"GOT >");
				send_command_to_esp(sbuffer, 1);
				send_command_to_esp("\r\n", 1);
			}			
						
			deleteString0();
			deleteString();
			del_esp_tx_ready();
		}	
		*/


//************************************************** Send command to UART0 **************************************************
/*
		if (get_computer_tx_ready() == 1){
			//const char * str_pc;
			//str_pc = getString();
			//send_string(str_pc);				// echo uart1
			send_string0(getString());			// send to uart0
			deleteString();						// delete uart1 buffer
			del_computer_tx_ready();		
		}	

*/
//************************************************** Read UART0 Response line by line **************************************************
/*
		if (get_esp_tx_ready() == 1){	
			const char * str_sim;
			str_sim = getString0();
			send_string(str_sim);					
			del_esp_tx_ready();
		}
*/


//************************************************************* SET MENU *************************************************************
		if (button_scan_cycle == 25000){

			if((BTN_UP == 1) & (buttonDown == 0)){

				if (systemState == Idle) {
					thrmSetDaysMain.state = 1;
					eeprom_write_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));
				}
				
				if (systemState == Edit) {
					engine->navigate(engine->getPrev());
				
					updateMenu = 1;
					setItemNr = 0;
					thProgram = 1;
				}							
			}
		
			if((BTN_LEFT == 1) & (buttonDown == 0)){
				
				if (systemState == Idle) {
					thrmSetDaysMain.state = 4;
					eeprom_write_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));
				}				
				
				if (systemState == Edit) {
					engine->navigate(engine->getParent());
				
					updateMenu = 1;
					setItemNr = 0;
					thProgram = 1;
				}
							
			}

			if((BTN_CENTER == 1) & (buttonDown == 0)){
				if ((systemState != EditTime) && (systemState != EditDate) && (systemState != EditThrmMon) 
					&& (systemState != EditThrmTue) && (systemState != EditThrmWed) && (systemState != EditThrmThu) 
					&& (systemState != EditThrmFri) && (systemState != EditThrmSat) && (systemState != EditThrmSun)
					&& (systemState != EditThermostatState) && (systemState != EditDeleteSMS) && (systemState != EditReadSMS)) {
					engine->navigate(&miEditTimeDate);

					systemState = Edit;
					previousSystemState = systemState;				
					updateMenu = 1;
					setItemNr = 0;
					thProgram = 1;
				}					
			}

			if((BTN_RIGHT == 1) & (buttonDown == 0)){
				if (systemState == Idle) {
					thrmSetDaysMain.state = 2;
					eeprom_write_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));
				}				
				if (systemState == Edit) {
					//engine->navigate(engine->getChild());
					engine->invoke();							// ha nincs child akkor a hivja az actiont
					updateMenu = 1;
					setItemNr = 0;
					thProgram = 1;
				}				
			}
		
		
			if((BTN_DOWN == 1) & (buttonDown == 0)){
				if (systemState == Idle) {
					thrmSetDaysMain.state = 3;
					eeprom_write_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));
				}				
				if (systemState == Edit) {
					engine->navigate(engine->getNext());
				
					updateMenu = 1;
					setItemNr = 0;
					thProgram = 1;
				}				
			}
		
		
			if ((systemState == EditThrmMon) || (systemState == EditThrmTue) || (systemState == EditThrmWed) || (systemState == EditThrmThu) || (systemState == EditThrmFri) || (systemState == EditThrmSat) || (systemState == EditThrmSun)) {
				while(1){
					uint8_t thDay = systemState - 5;
					enable_update_relay = 0;					
			
					if (setItemNr != prevSetItemNr){
						lcd_clrscr();
				
						lcd_gotoxy(0,0);
						sprintf(rtcBuffer, "Edit %s", getLabelSetThrmDay(systemState));
						lcd_puts(rtcBuffer);
				
						prevSetItemNr = setItemNr;
						if (systemState != previousSystemState){
							eeprom_read_block(&thrmSetDaysMain, &EEthrmSetDays, sizeof(thrmSetDaysMain));			// betolti az EEPROM bol a beallitasokat
							previousSystemState = systemState;
						}
				
					}

					if((BTN_CENTER == 1) & (buttonDown == 0)){
						engine->navigate(engine->getParent());
						updateMenu = 1;
						systemState = Edit;
						previousSystemState = systemState;
						eeprom_write_block(&thrmSetDaysMain.therm_day[thDay], &EEthrmSetDays.therm_day[thDay], sizeof(thrmSetDaysMain.therm_day[thDay]));				// menti EEPROM ba a beallitasokat
						break;
					}

					if((BTN_LEFT == 1) & (buttonDown == 0)) setItemNr--;
					if((BTN_RIGHT == 1) & (buttonDown == 0)) setItemNr++;
														
			
					if((BTN_UP == 1) & (buttonDown == 0)){
						if(setItemNr == 1){
							if (thProgram <= 6) thProgram++;
						}	
				
						if (enablePgmSet(thrmSetDaysMain, thDay, thProgram, 1, setItemNr)){
							if(setItemNr == 2){
								if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour < 23) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour++;
							}
							if(setItemNr == 3){
								if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin < 59) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin++;
								if ((thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin == 59) && (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour < 23)){
									thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin = 0;
									thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour++;						
								}
							}
						}
				
						if(setItemNr == 4){
							if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp < 100) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp += 0.5;
						}			
					}

					if((BTN_DOWN == 1) & (buttonDown == 0)){
						if(setItemNr == 1){
							if (thProgram > 1) thProgram--;
						}		
								
						if (enablePgmSet(thrmSetDaysMain, thDay, thProgram, 0, setItemNr)){
							if(setItemNr == 2){
								if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour > 0) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour--;
							}
							if(setItemNr == 3){						
								if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin >= 0) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin--;
								if ((thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin == 255) && (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour > 0)){
									thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin = 59;
									thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour--;	
								}
							}
						}
				
						if(setItemNr == 4){
							if (thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp > -50) thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp -= 0.5;
						}				
					}


					int ptlefts = (int)thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp;
					int ptrights = abs(((int)(thrmSetDaysMain.therm_day[thDay].program[thProgram-1].ptemp * 10)) % 10);			
					sprintf(rtcBuffer, "P%d %.2d:%.2d %.2d.%dC   ", thProgram, thrmSetDaysMain.therm_day[thDay].program[thProgram-1].phour, thrmSetDaysMain.therm_day[thDay].program[thProgram-1].pmin, ptlefts, ptrights);
					lcd_gotoxy(0,1);
					lcd_puts(rtcBuffer);
			

					if (setItemNr < 0) setItemNr = 0;
					if (setItemNr > 4) setItemNr = 4;
			
					if(((BTN_LEFT == 1) & (buttonDown == 0))  && (setItemNr == 0)){
						engine->navigate(engine->getParent());
						updateMenu = 1;
						systemState = Edit;
						previousSystemState = systemState;
						eeprom_write_block(&thrmSetDaysMain.therm_day[thDay], &EEthrmSetDays.therm_day[thDay], sizeof(thrmSetDaysMain.therm_day[thDay]));
						break;				
					}
			
					if (thProgram < 1) thProgram = 1;
					if (thProgram > 6) thProgram = 6;
							
				
					if(setItemNr == 1){
						sprintf(rtcBuffer, "^");
						lcd_gotoxy(1,2);
					}
					if(setItemNr == 2){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(3,2);
					}
					if(setItemNr == 3){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(6,2);
					}
					if(setItemNr == 4){
						sprintf(rtcBuffer, "^^ ^");
						lcd_gotoxy(9,2);
					}
					lcd_puts(rtcBuffer);						
				
					buttonDown = ((BTN_UP == 1) | (BTN_DOWN == 1) | (BTN_LEFT == 1) | (BTN_RIGHT == 1) | (BTN_CENTER == 1));
					delay_ms(4);				
				
				}

			}	
		
			
		
			if (systemState == EditTime) {
				while(1){
					if (setItemNr != prevSetItemNr){
						lcd_clrscr();				
						lcd_gotoxy(0,0);
						sprintf(rtcBuffer, "Set-%s", getLabelSetTimeHourMinute(setItemNr));
						lcd_puts(rtcBuffer);												
						prevSetItemNr = setItemNr;
					}

					if((BTN_CENTER == 1) & (buttonDown == 0)){
						engine->navigate(engine->getParent());				
						updateMenu = 1;
						systemState = Edit;
						previousSystemState = systemState;
						break;
					}

					if((BTN_LEFT == 1) & (buttonDown == 0)) setItemNr--;
					if((BTN_RIGHT == 1) & (buttonDown == 0)) setItemNr++;

					if((BTN_UP == 1) & (buttonDown == 0)){
						if(setItemNr == 1){					
							if (rtctime->hour < 23) rtctime->hour++;
						}
						if(setItemNr == 2){
							if (rtctime->min < 59) rtctime->min++;
						}
						rtc_set_time(rtctime);
					}
			
					if((BTN_DOWN == 1) & (buttonDown == 0)){
						if(setItemNr == 1){					
							if (rtctime->hour > 0) rtctime->hour--;
						}
						if(setItemNr == 2){
							if (rtctime->min > 0) rtctime->min--;					
						}
						rtc_set_time(rtctime);
					}


					sprintf(rtcBuffer, "Time: %.2d:%.2d", rtctime->hour, rtctime->min);
					lcd_gotoxy(0,1);
					lcd_puts(rtcBuffer);

					if (setItemNr < 1) setItemNr = 1;
					if (setItemNr > 2) setItemNr = 2;

					if(setItemNr == 1){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(6,2);
					}
					if(setItemNr == 2){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(9,2);
					}
					lcd_puts(rtcBuffer);
				
					buttonDown = ((BTN_UP == 1) | (BTN_DOWN == 1) | (BTN_LEFT == 1) | (BTN_RIGHT == 1) | (BTN_CENTER == 1));
					delay_ms(5);
				
				}

			}		
		
			if (systemState == EditDate) {
				while(1){
					if (setItemNr != prevSetItemNr){
						lcd_clrscr();
						lcd_gotoxy(0,0);
						sprintf(rtcBuffer, "Set-%s", getLabelSetTimeDate(setItemNr));
						lcd_puts(rtcBuffer);
						prevSetItemNr = setItemNr;
					}
					if((BTN_CENTER == 1) & (buttonDown == 0)){
						engine->navigate(engine->getParent());
						updateMenu = 1;
						systemState = Edit;
						previousSystemState = systemState;
						break;
					}
			
					if((BTN_LEFT == 1) & (buttonDown == 0)) setItemNr--;
					if((BTN_RIGHT == 1) & (buttonDown == 0)) setItemNr++;			
			
					if((BTN_UP == 1) & (buttonDown == 0)){
						if(setItemNr == 1){
							if (rtctime->year < 99) rtctime->year++;
						}
						if(setItemNr == 2){
							if (rtctime->mon < 12) rtctime->mon++;
						}
						if(setItemNr == 3){
							if (rtctime->mday < month_daynum_arr[rtctime->mon-1]) rtctime->mday++;
							if (rtctime->mday > month_daynum_arr[rtctime->mon-1]) rtctime->mday = month_daynum_arr[rtctime->mon-1];
						}	
						if(setItemNr == 4){
							if (rtctime->wday < 7) rtctime->wday++;
						}				
						rtc_set_time(rtctime);			
					}
			
					if((BTN_DOWN == 1) & (buttonDown == 0)){
						if(setItemNr == 1){
							if (rtctime->year > 1) rtctime->year--;
						}
						if(setItemNr == 2){
							if (rtctime->mon > 1) rtctime->mon--;
						}
						if(setItemNr == 3){
							if (rtctime->mday > 1) rtctime->mday--;
						}	
						if(setItemNr == 4){
							if (rtctime->wday > 1) rtctime->wday--;
						}							
						rtc_set_time(rtctime);
					}	
							
					sprintf(rtcBuffer, "Date: 20%.2d/%.2d/%.2d %s", rtctime->year, rtctime->mon, rtctime->mday, rtc_get_weekday(rtctime->wday));
					lcd_gotoxy(0,1);
					lcd_puts(rtcBuffer);
			
					if (setItemNr < 1) setItemNr = 1;
					if (setItemNr > 4) setItemNr = 4;			
						
					if(setItemNr == 1){
						sprintf(rtcBuffer, "^^^^");
						lcd_gotoxy(6,2);				
					}
					if(setItemNr == 2){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(11,2);				
					}
					if(setItemNr == 3){
						sprintf(rtcBuffer, "^^");
						lcd_gotoxy(14,2);
					}
					if(setItemNr == 4){
						sprintf(rtcBuffer, "^^^");
						lcd_gotoxy(17,2);
					}
					lcd_puts(rtcBuffer);
				
					buttonDown = ((BTN_UP == 1) | (BTN_DOWN == 1) | (BTN_LEFT == 1) | (BTN_RIGHT == 1) | (BTN_CENTER == 1));
					delay_ms(5);				
				}			
			}		

			if (systemState == EditDeleteSMS) {
				send_string0("AT+CMGF=1\r\n");
				delay_ms(10);				
				send_string(getString0());
				deleteString0();
				send_string("\nSMS text mode\n");
								
				send_string0("AT+CMGDA=DEL ALL\r\n");
				delay_ms(10);
				send_string(getString0());
				deleteString0();
				send_string("\nAll SMS Deleted\n");
				
				Sim8.ResetNrOfSms();
				
				engine->navigate(engine->getParent());
				updateMenu = 1;
				systemState = Edit;
				previousSystemState = systemState;				
			}
			
/*############ Read SMS ###############*/
			
			if (systemState == EditReadSMS) {
				if(displ_count > 10){									
					char buf[21];	
					last_sms_addr = get_last_txt_address();									
					snprintf(buf, sizeof(buf), "SMS %i Last: %i", eeprom_address_read, last_sms_addr);
					lcd_gotoxy(0,0);
					lcd_puts(buf);
									
					displ_count = 0;
				}
				
				if((BTN_UP == 1) & (buttonDown == 0)){
					if (eeprom_address_read < 4020){
						eeprom_address_read += 60;
						readSMS = 1;												
					}
				}
				
				if((BTN_DOWN == 1) & (buttonDown == 0)){
					if (eeprom_address_read >= 120){
						eeprom_address_read -= 60;
						readSMS = 1;						
					}					
				}				
				
				if((BTN_CENTER == 1) & (buttonDown == 0)){
					engine->navigate(engine->getParent());
					updateMenu = 1;
					systemState = Edit;
					previousSystemState = systemState;				
				}
			}			
			
			if ((check_sms_and_update_busy == 0) && (readSMS == 1)){
				
				send_string("\nBUTTON TEST BEGIN\n");
						
/*						
				send_string0("AT+CMGL=\"ALL\"\r\n");				
				deleteString0();
				delay_ms(100);
				send_string(getString0());				
				deleteString0();
*/				
				readSMS = 0;
								
				lcd_clrscr();														
				
				char sms_buf[21];						
				strcpy(sms_buf, "");
				snprintf(sms_buf, sizeof(sms_buf), "%s", readEEPROM(eeprom_address_read, 20));
				lcd_gotoxy(0,1);
				lcd_puts(sms_buf);				
				
				strcpy(sms_buf, "");
				snprintf(sms_buf, sizeof(sms_buf), "%s", readEEPROM(eeprom_address_read + 20, 20));
				lcd_gotoxy(0,2);
				lcd_puts(sms_buf);
				
				strcpy(sms_buf, "");
				snprintf(sms_buf, sizeof(sms_buf), "%s", readEEPROM(eeprom_address_read + 40, 20));
				lcd_gotoxy(0,3);
				lcd_puts(sms_buf);								
														
				
				if (last_sms_addr > 3840){
					set_last_txt_address(0);
				}
				
								
				send_string("\nBUTTON TEST END\n");				
			}
			
/*############ Read SMS ###############*/			

			if (systemState == EditThermostatState) {
				char manualtemp_buffer[20];
				* manualtemp_buffer = '\0';
				bool button_enable_EditThermostatState;			// hogy ne inkrementelodjon egybol a setItemNr amikor kivalasztjuk a menut
				while(1){				
					if (systemState != previousSystemState){						
						eeprom_read_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));			// betolti az EEPROM bol a beallitasokat
						setItemNr = thrmSetDaysMain.state;
						previousSystemState = systemState;			
						
						lcd_clrscr();
									
						sprintf(rtcBuffer, "Set State     ");
						lcd_gotoxy(0,0);
						lcd_puts(rtcBuffer);
						
						sprintf(rtcBuffer, "Auto Manual Off ");						
						lcd_gotoxy(0,1);
						lcd_puts(rtcBuffer);	
						
						lcd_gotoxy(16,1);
						lcd_data((uint8_t)IconLeftHalfStar);
						
						lcd_gotoxy(17,1);
						lcd_data((uint8_t)IconRightHalfStar);	
						
						lcd_gotoxy(18,1);
						lcd_putc(' ');

						lcd_gotoxy(19,1);
						lcd_putc(' ');						
						
						button_enable_EditThermostatState = false;					
					}

					if((BTN_CENTER == 1) & (buttonDown == 0)){
						engine->navigate(engine->getParent());
						updateMenu = 1;
						systemState = Edit;
						previousSystemState = systemState;
						thrmSetDaysMain.state = setItemNr;
						eeprom_write_block(&thrmSetDaysMain.state, &EEthrmSetDays.state, sizeof(thrmSetDaysMain.state));
						eeprom_write_block(&thrmSetDaysMain.manual_temp, &EEthrmSetDays.manual_temp, sizeof(thrmSetDaysMain.manual_temp));
						break;
					}


					if((BTN_LEFT == 1) & (buttonDown == 0)) setItemNr--;
					if((BTN_RIGHT == 1) & (buttonDown == 0) & (button_enable_EditThermostatState == true)) setItemNr++;

					if (setItemNr < 1) setItemNr = 1;
					if (setItemNr > 4) setItemNr = 4;

					if(setItemNr == 1){						
										 //"Auto Manual Off **  "
						sprintf(rtcBuffer, "----                ");						
					}
					if(setItemNr == 2){						
										 
						if((BTN_UP == 1) & (buttonDown == 0)) thrmSetDaysMain.manual_temp += 0.1;
						if((BTN_DOWN == 1) & (buttonDown == 0)) thrmSetDaysMain.manual_temp -= 0.1;
										 //"Auto Manual Off **  "
						sprintf(rtcBuffer, "     ------         ");												
						int mtemplefts = (int)thrmSetDaysMain.manual_temp;
						int mtemprights = abs(((int)(thrmSetDaysMain.manual_temp * 10)) % 10);						
						sprintf(manualtemp_buffer, "     %.2d.%d", mtemplefts, mtemprights);									
						
					}
					if(setItemNr == 3){						
										 //"Auto Manual Off **  "						
						sprintf(rtcBuffer, "            ---     ");						
					}
					if(setItemNr == 4){
										 //"Auto Manual Off **  "
						sprintf(rtcBuffer, "                --  ");
					}																																		
					
					lcd_gotoxy(0,2);
					lcd_puts(rtcBuffer);
					
					lcd_gotoxy(0,3);
					lcd_puts(manualtemp_buffer);	
					
					lcd_gotoxy(9,3);
					lcd_data((uint8_t)IconGrade);
						
					lcd_gotoxy(10,3);
					sprintf(sbuffer, "C         ");
					lcd_puts(sbuffer);									
															
					buttonDown = ((BTN_UP == 1) | (BTN_DOWN == 1) | (BTN_LEFT == 1) | (BTN_RIGHT == 1) | (BTN_CENTER == 1));
					delay_ms(10);	
					button_enable_EditThermostatState = true;
				}
			}

			if (updateMenu == 1) {
				updateMenu = 0;

				lcd_clrscr();
				// render the menu
				engine->render(renderMenuItem, menuItemsVisible);

			}

			if (systemState == Idle) {
				if (systemState != previousSystemState) {
					lcd_clrscr();
					previousSystemState = systemState;
				
				}

				if(displ_count > 10){
					lcd_home();									
					//lcd_puts("Main Screen...");						
			
					ftemp = ds1820_read_temp(DS1820_pin_sens);
					int lefts = (int)ftemp;
					int rights = abs(((int)(ftemp * 10)) % 10);
					eeprom_read_block(&thrmSetDaysMain, &EEthrmSetDays, sizeof(thrmSetDaysMain));
																	 				
					//T23.4gC
					
					lcd_gotoxy(0,0);
					lcd_data((uint8_t)IconThermometer);					
									
					if(display_temp_refresh_ct > 4){	
						lcd_gotoxy(1,0);
						sprintf(sbuffer, "%.2d.%d", lefts, rights);
						lcd_puts(sbuffer);
						display_temp_refresh_ct = 0;
					}
					
					lcd_gotoxy(6,0);
					lcd_putc('C');
					
					lcd_gotoxy(5,0);
					lcd_data((uint8_t)IconGrade);	
					
					if(thrmSetDaysMain.state == 1){
						lcd_gotoxy(14,0);
						lcd_puts("  Auto");
					}
					if(thrmSetDaysMain.state == 2){
						lcd_gotoxy(14,0);
						lcd_puts("Manual");
					}
					if(thrmSetDaysMain.state == 3){
						lcd_gotoxy(14,0);
						lcd_puts("   Off");
					}														
					if(thrmSetDaysMain.state == 4){	
						lcd_gotoxy(14,0);
						lcd_puts("      ");						
											
						lcd_gotoxy(18,0);
						lcd_data((uint8_t)IconLeftHalfStar);
						
						lcd_gotoxy(19,0);
						lcd_data((uint8_t)IconRightHalfStar);
					}				
										
					put_temp_prog_data();			
					
					lcd_gotoxy(0,2);
					lcd_puts("  ");
					lcd_gotoxy(0,2);
					lcd_puts(Sim8.GetNrOfSms());
					
					lcd_gotoxy(2,2);		
					lcd_puts("SMS");				
					
					rtctime = rtc_get_time();				
					sprintf(rtcBuffer, "%.2d:%.2d 20%.2d/%.2d/%.2d %s", rtctime->hour, rtctime->min, rtctime->year, rtctime->mon, rtctime->mday, rtc_get_weekday(rtctime->wday));
					lcd_gotoxy(0,3);
					lcd_puts(rtcBuffer);
				
					displ_count = 0;
					setItemNr = 0;
					enable_update_relay = 1;
				}
			}			
			displ_count++;				
											
		
			buttonDown = ((BTN_UP == 1) | (BTN_DOWN == 1) | (BTN_LEFT == 1) | (BTN_RIGHT == 1) | (BTN_CENTER == 1));
			button_scan_cycle = 0;
		
		}
		button_scan_cycle++;
		
//************************************************************* END SET MENU *************************************************************		

		if (sec_refresh == 1){
			sec_refresh = 0;
		}	


  	}
	  	  
}
						
											
