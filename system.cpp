
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "bits.h"
#include "delay.h"
#include "lcd.h"
#include "rtc.h"
#include "ds18x20lib.h"
#include "string.h"
#include "Sim800l.h"
#include "twi.h"
#include "eeprom.h"

#define		BUZZ1_HIGH		PORTE |= BIT4 
#define		BUZZ1_LOW		PORTE &= ~BIT4 

#define		BUZZ2_HIGH		PORTE |= BIT5 
#define		BUZZ2_LOW		PORTE &= ~BIT5 
#define		BUFFERSIZE		128

#define 	RELAY_HIGH		PORTA |= BIT6
#define 	RELAY_LOW		PORTA &= ~BIT6


void send_char(char c);
static char buffer0[BUFFERSIZE];
volatile uint8_t nr_read0;
static char buffer[BUFFERSIZE];
static uint8_t nr_read;
static uint8_t esp_tx_ready;
static uint8_t computer_tx_ready;
struct tm* rtctime = NULL;
thermostatSetDays EEMEM EEthrmSetDays = {};	
thermostatSetDays thrmSetDays = {};
	
float phour_from = 0.0;
float phour_to = 0.0;
float actual_phour_from = 0.0;
float actual_phour_to = 0.0;
uint8_t actual_prog = 0;
float ptemp = 0.0;
uint8_t enable_update_relay = 0;
uint8_t check_sms_and_update_busy = 0;
Sim800l Sim8;
uint8_t display_temp_refresh_ct = 0;
char sendstr[100];
const char * message_in;
const char * msg_id;


//--------Init Ports --------------------
void InitPorts(void) {

	// Input/Output Ports initialization
	// Port A initialization
	PORTA=0x80;
	DDRA=0x40;

	// Port B initialization
	PORTB=0x00;
	DDRB=0x00;

	// Port C initialization
	PORTC=0x00;
	DDRC=0xF7;

	// Port D initialization
	PORTD=0x00;
	DDRD=0x08;

	// Port E initialization
	PORTE=0x00;
	DDRE=0x30;

	// Port F initialization
	PORTF=0x00;
	DDRF=0x00;

	// Port G initialization
	PORTG=0x00;
	DDRG=0x00;

}

//--------Init Timers --------------------
void InitTimers(void) {

	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: Timer 0 Stopped
	// Mode: Normal top=FFh
	// OC0 output: Disconnected
	///ASSR=0x00;
	///TCCR0=0x00;
	///TCNT0=0x00;
	///OCR0=0x00;

	// Timer/Counter 1 initialization
	// Clock source: T1 pin Rising Edge
	// Mode: Normal top=FFFFh
	// OC1A output: Discon.
	// OC1B output: Discon.
	// OC1C output: Discon.
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	TCCR1A=0x00;
	TCCR1B=0x07;
	TCNT1H=0x00;
	TCNT1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x00;
	OCR1BH=0x00;
	OCR1BL=0x00;
	OCR1CH=0x00;
	OCR1CL=0x00;

	// Timer/Counter 2 initialization
	// Clock source: T2 pin Rising Edge
	// Mode: Normal top=FFh
	// OC2 output: Disconnected
	TCCR2=0x07;
	TCNT2=0x00;
	OCR2=0x00;

	// Timer/Counter 3 initialization
	// Clock source: System Clock
	// Clock value: 2000,000 kHz
	// Mode: Normal top=FFFFh
	// OC3A output: Discon.
	// OC3B output: Toggle
	// OC3C output: Toggle
	//TCCR3A=0x14;
	//TCCR3B=0x02;
	//TCNT3H=0x00;
	//TCNT3L=0x00;
	//OCR3AH=0x00;
	//OCR3AL=0x00;
	//OCR3BH=0x00;
	//OCR3BL=0xFA;
	//OCR3CH=0x00;
	//OCR3CL=0xFA;

	// External Interrupt(s) initialization
	// INT0: Off
	// INT1: Off
	// INT2: Off
	// INT3: Off
	// INT4: Off
	// INT5: Off
	// INT6: Off
	// INT7: Off
	///EICRA=0x00;
	///EICRB=0x00;
	///EIMSK=0x00;

	// Timer(s)/Counter(s) Interrupt(s) initialization
	///TIMSK=0x00;
	///ETIMSK=0x00;

} 

//--------Init Uart interface --------------------

void InitUart0(void) {

	// USART0 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART0 Receiver: On
	// USART0 Transmitter: On
	// USART0 Mode: Asynchronous
	// USART0 Baud rate: 115200
	UCSR0A=0x00;
	UCSR0B=0x18;     		//(00011000)
	UCSR0C=0x06;
	UBRR0H=0x00;
	UBRR0L=0x07;			//7
	//UCSR0B |= 1<<TXCIE0;	   // turn on Receive Complete interrupt vector
	//UCSR0B |= 1<<UDRIE0;    // keep off Data Register Empty interrupt vector
	UCSR0B |= (1 << RXCIE0); 		// Enable the USART Recieve Complete interrupt (USART_RXC)
	
	esp_tx_ready = 0;

}

void InitUart1(void) {

	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud rate: 115200
	
	UCSR1A=0x00;
	UCSR1B=0x18;     		//(00011000)
	UCSR1C=0x06;
	UBRR1H=0x00;
	UBRR1L=0x07;	
	UCSR1B |= (1 << RXCIE1); 									// Enable the USART Recieve Complete interrupt (USART_RXC)
	sei();													// Enable the Global Interrupt Enable flag so that interrupts can be processed


}

void SendCharUart0(unsigned char ch) {

	// wait for data to be received
	while(!(UCSR0A & (1<<UDRE0)));
	// send data
	UDR0 = ch; 
	
}

void SendCharUart1(unsigned char ch) {

	// wait for data to be received
	while(!(UCSR1A & (1<<UDRE1)));
	// send data
	UDR1 = ch; 
	
}

unsigned char ReceiveCharUart1(void) {
	
	// wait for data to be received
	while(!(UCSR1A & (1<<RXC1)));
	// get and return received data from buffer
	return UDR1; 

}

unsigned char ReceiveCharUart1_nonstop(void) {
	
	// wait for data to be received
	if((UCSR1A & (1<<RXC1)))
		// get and return received data from buffer
		return UDR1; 
	else
		// return 0
		return 0;

}

unsigned char ReceiveCharUart0_nonstop(void) {
	
	// wait for data to be received
	if((UCSR0A & (1<<RXC0)))
		// get and return received data from buffer
		return UDR0;
	else
		// return 0
		return 0;

}

void send_string0(const char s[])
{	
	uint8_t i =0;
	
	while (s[i] != 0x00)
	{
		SendCharUart0(s[i]);
		i++;
	}
}

void send_string(const char s[])
{
	uint8_t i = 0;
	
	while (s[i] != 0x00)
	{
		SendCharUart1(s[i]);
		i++;
	}
}

const char * getString(void){
	return (const char *)buffer;
}

const char * getString0(void){
	return (const char *)buffer0;
}

uint8_t get_esp_tx_ready(void){
	return esp_tx_ready;	
}

void del_esp_tx_ready(void){
	esp_tx_ready = 0;
}

uint8_t get_computer_tx_ready(void){
	return computer_tx_ready;
}

void del_computer_tx_ready(void){
	computer_tx_ready = 0;
}

void deleteString0(void){
	uint8_t i = BUFFERSIZE + 1;
	while (i){
		buffer0[--i] = 0;
	}
	nr_read0 = 0;
}

void deleteString(void){
	uint8_t i = BUFFERSIZE + 1;
	while (i){
		buffer[--i] = 0;
	}	
	nr_read = 0;
}

//////////////////////////////////////Interrupts
ISR(USART1_RX_vect)
{
	unsigned char str_in;
	// Get data from the USART in register
	str_in = UDR1;
	if ((nr_read >= 0) && (nr_read <= BUFFERSIZE)){
		buffer[nr_read++] = str_in;
	}

	if (str_in == '\n'){
		computer_tx_ready = 1;
	}
	else computer_tx_ready = 0;
	
}

ISR(USART0_RX_vect)
{
	unsigned char c_in;
	c_in = UDR0;
	if ((nr_read0 >= 0) && (nr_read0 <= BUFFERSIZE)){
		buffer0[nr_read0++] = c_in;	
	}
	
	//az OK-t lehetne nézni
	
	if (c_in == '\n'){
		esp_tx_ready = 1;
	}
	else esp_tx_ready = 0;
	

}

//---------Buzzer --------------------------------- 
void Buzzer(void) {
  
	
	BUZZ1_LOW;			
	BUZZ2_HIGH;
	_delay_us(200);
    BUZZ2_LOW;
	BUZZ1_HIGH;
	_delay_us(200);

}   

//--------- LED --------------------------------- 

void signalLedOn(void){			
	BUZZ1_HIGH;
}

void signalLedOff(void){
	BUZZ1_LOW;
}

void put_temp_prog_data(void){
	int ptemplefts = (int)ptemp;
	int ptemprights = abs(((int)(ptemp * 10)) % 10);	
	char sbuffer[21];
	if(thrmSetDays.state == 1){
		lcd_gotoxy(11,1);		
		sprintf(sbuffer, "P%d %.2d.%d", actual_prog, ptemplefts, ptemprights);
		lcd_puts(sbuffer);

		lcd_gotoxy(18,1);
		lcd_data((uint8_t)IconGrade);
		lcd_gotoxy(19,1);
		lcd_putc('C');					
	}
	
	if(thrmSetDays.state == 2){
		int mtemplefts = (int)thrmSetDays.manual_temp;
		int mtemprights = abs(((int)(thrmSetDays.manual_temp * 10)) % 10);
		lcd_gotoxy(11,1);
		sprintf(sbuffer, "   %.2d.%d", mtemplefts, mtemprights);
		lcd_puts(sbuffer);	
		
		lcd_gotoxy(18,1);
		lcd_data((uint8_t)IconGrade);
		lcd_gotoxy(19,1);
		lcd_putc('C');		
	}	
	
	if ((thrmSetDays.state == 3)){
		lcd_gotoxy(0,1);
		lcd_puts("                    ");
	}	
	
	if(thrmSetDays.state == 4){
		int mtemplefts = (int)thrmSetDays.idle_temp;
		int mtemprights = abs(((int)(thrmSetDays.idle_temp * 10)) % 10);
		lcd_gotoxy(11,1);
		sprintf(sbuffer, "   %.2d.%d", mtemplefts, mtemprights);
		lcd_puts(sbuffer);
		
		lcd_gotoxy(18,1);
		lcd_data((uint8_t)IconGrade);
		lcd_gotoxy(19,1);
		lcd_putc('C');		
	}		
}

/**
 * Replace all occurrence of a character in given string.
 */
void replaceAll(char * str, char oldChar, char newChar)
{
    int i = 0;

    /* Run till end of string */
    while(str[i] != '\0')
    {
        /* If occurrence of character is found */
        if(str[i] == oldChar)
        {
            str[i] = newChar;
        }

        i++;
    }
}

/**
 * Replace all occurrence of a character in given string.
 */
void removeChar(char * str, char remChar)
{
	
	
	//NEMJO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int i = 0;		

    /* Run till end of string */
    while(str[i] != '\0')
    {
        /* If occurrence of character is found */
        if(str[i] == remChar)
        {
			str[i] = str[++i];
			i--;
        }			
        i++;
    }
}


void save_sms(const char * msg_in){
	char savestr[100];
	char * comp_msg;
	
	send_string(">>==========\n");	
	send_string(msg_in);
	comp_msg = strstr(msg_in, "\r\n");	
	comp_msg = strstr(comp_msg+2, "\r\n");
/*
	int length;
	length = strlen(comp_msg);
	if (length > 7) comp_msg[length - 7] = '\0';
	*/	

	snprintf(savestr, sizeof(savestr), "%s", comp_msg);	

	send_string("SAVE STR>>>\n");
	
	removeChar(savestr, 10);			// \n
	removeChar(savestr, 13);			// \r		
	
	send_string(savestr);
	send_string("\nSAVE STR<<<\n");

		
	serial_eeprom_write_block(savestr);
	
	send_string(">>----------\n");
		
}


void CheckTemperatureAndUpdateRelay(void){	
	
	check_sms_and_update_busy = 1;
	display_temp_refresh_ct++;	
	
	float ftemp = ds1820_read_temp(DS1820_pin_sens);

	if (enable_update_relay){
		eeprom_read_block(&thrmSetDays, &EEthrmSetDays, sizeof(thrmSetDays));	
		float now = (float)rtctime->hour + ((float)rtctime->min / 100);
		for(uint8_t prog = 0; prog <= 5; prog++){
			
			if (prog == 0) phour_from = 0.0;
			else phour_from = (float)thrmSetDays.therm_day[rtctime->wday-1].program[prog].phour + ((float)thrmSetDays.therm_day[rtctime->wday-1].program[prog].pmin / 100);
			
			if (prog == 5) phour_to = 23.59;
			else phour_to = (float)thrmSetDays.therm_day[rtctime->wday-1].program[prog+1].phour + ((float)thrmSetDays.therm_day[rtctime->wday-1].program[prog+1].pmin / 100);				
		
			if ((phour_from <= now) && (now <= phour_to)){
				actual_phour_from = phour_from;
				actual_phour_to = phour_to;
				actual_prog = prog+1;
				ptemp = thrmSetDays.therm_day[rtctime->wday-1].program[prog].ptemp;											
						
				
				if(thrmSetDays.state == 1){
					if ((ftemp + 0.5) <= ptemp){
						RELAY_HIGH;
					}
					if ((ftemp - 0.5) >= ptemp){
						RELAY_LOW;
					}
				}
					
				if(thrmSetDays.state == 2){

					if ((ftemp + 0.5) <= thrmSetDays.manual_temp){
						RELAY_HIGH;
					}
					if ((ftemp - 0.5) >= thrmSetDays.manual_temp){
						RELAY_LOW;
					}
				}
					
				if(thrmSetDays.state == 3){
					RELAY_LOW;
				}
											
				if(thrmSetDays.state == 4){
					if ((ftemp + 0.5) <= thrmSetDays.idle_temp){
						RELAY_HIGH;
					}
					if ((ftemp - 0.5) >= thrmSetDays.idle_temp){
						RELAY_LOW;
					}
				}													
			}					
		}		
	}	
	
	msg_id = Sim8.checkSms();
	if (strstr(msg_id, "no sms") == NULL){

		delay_ms(10);
		Sim8.readSms(msg_id);
		delay_ms(10);
		message_in = getString0();
		
		save_sms(message_in);

		if ((strstr(message_in, STR_STATE_ON_AUTO) != 0) || (strstr(message_in, STR_STATE_ON_MAUAL) != 0) ||
		(strstr(message_in, STR_STATE_OFF) != 0) || (strstr(message_in, STR_TEMP_MAN_MODIF) != 0) ||
		(strstr(message_in, SMS_TEMP_QUERY) != NULL) || (strstr(message_in, STR_STATE_IDLE) != NULL)){
			if (strstr(message_in, STR_STATE_ON_AUTO) != 0){
				thrmSetDays.state = 1;
				snprintf(sendstr, sizeof(sendstr), "A termosztat Auto-ra beallitva.");
			}
			if (strstr(message_in, STR_STATE_ON_MAUAL) != 0){
				thrmSetDays.state = 2;
				snprintf(sendstr, sizeof(sendstr), "A termosztat Manual-ra beallitva.");
			}
			if (strstr(message_in, STR_STATE_OFF) != 0){
				thrmSetDays.state = 3;
				snprintf(sendstr, sizeof(sendstr), "A termosztat kikapcsolva.");
			}

			if (strstr(message_in, STR_STATE_IDLE) != NULL){
				thrmSetDays.state = 4;
				snprintf(sendstr, sizeof(sendstr), "A homerseklet beallitva 5 C fokra.");
			}
			if (strstr(message_in, STR_TEMP_MAN_MODIF) != NULL){
				char * start;
				start = strstr(message_in, "==");
				
				float manual_temp;
				manual_temp = 00.0;
				manual_temp += atoi(start+2);
				manual_temp += atoi(start+5) * 0.1;
				
				snprintf(sendstr, sizeof(sendstr), "A homerseklet beallitva: %c%c.%c", start[2], start[3], start[5]);
				thrmSetDays.manual_temp = manual_temp;
				eeprom_write_block(&thrmSetDays.manual_temp, &EEthrmSetDays.manual_temp, sizeof(thrmSetDays.manual_temp));
			}
			
			if (strstr(message_in, SMS_TEMP_QUERY) != NULL){
				int mtemplefts = (int)ftemp;
				int mtemprights = abs(((int)(ftemp * 10)) % 10);
				snprintf(sendstr, sizeof(sendstr), "A homerseklet %.2d.%d C fok.", mtemplefts, mtemprights);
			}
			
			eeprom_write_block(&thrmSetDays.state, &EEthrmSetDays.state, sizeof(thrmSetDays.state));
			Sim8.SendSms(sendstr);
		}
		
		deleteString0();
	}	
	
	check_sms_and_update_busy = 0;
}
	
		
//*****************************************************************************************************************************

bool enablePgmSet(thermostatSetDays days_mem, uint8_t nr_day, uint8_t nr_prog, uint8_t up, uint8_t itemnr){
	
	// a kijelzon a nr_prog = n, a rendszerben n - 1 
	
	float phourfrom;
	float phourto;
	float phouract = (float)days_mem.therm_day[nr_day].program[nr_prog-1].phour + ((float)days_mem.therm_day[nr_day].program[nr_prog-1].pmin / 100);		
			

	if (nr_prog == 6){
		phourfrom = (float)days_mem.therm_day[nr_day].program[nr_prog-2].phour + ((float)days_mem.therm_day[nr_day].program[nr_prog-2].pmin / 100);
		phourto = 23.59;
		if (itemnr == 2) phourfrom++;
	}
	else{
		if (nr_prog == 1){
			phourfrom = 0.0;
			phourto = (float)days_mem.therm_day[nr_day].program[nr_prog].phour + ((float)days_mem.therm_day[nr_day].program[nr_prog].pmin / 100);				
			if (itemnr == 2) phourto--;
		}
		else{
			phourfrom = (float)days_mem.therm_day[nr_day].program[nr_prog-2].phour + ((float)days_mem.therm_day[nr_day].program[nr_prog-2].pmin / 100);
			phourto = (float)days_mem.therm_day[nr_day].program[nr_prog].phour + ((float)days_mem.therm_day[nr_day].program[nr_prog].pmin / 100);
			if (itemnr == 2){
				phourfrom++;
				phourto--;				
			}
		}
	}
	
	if(up){
		if (phouract < phourto) return true;
		else return false;		
	}
	else{
		if (phourfrom < phouract) return true;
		else return false;
	}

	
}

//******************************************** Build custom characters ********************************************
void createCustomChars(void){

	LCD_create_custom_char(IconLeft, left);
	LCD_create_custom_char(IconRight, right);
	LCD_create_custom_char(IconBack, back);
	LCD_create_custom_char(IconBlock, block);	
	LCD_create_custom_char(IconThermometer, thermometer);
	LCD_create_custom_char(IconGrade, grade);
	LCD_create_custom_char(IconLeftHalfStar, left_half_star);
	LCD_create_custom_char(IconRightHalfStar, right_half_star);	

}