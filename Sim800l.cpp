/* 
*/
#include <stdlib.h>
#include <stdio.h>
#include "Sim800l.h"
#include "system.h"
#include "string.h"
#include "bits.h"
#include "delay.h"
#include "trex.h"
#include "lcd.h"


//
//PUBLIC METHODS
//

#ifdef _UNICODE
#define trex_sprintf swprintf
#else
#define trex_sprintf sprintf
#endif


void Sim800l::InitSIM(void){
	send_string("INIT SIM\n");		
		
	uint8_t nr_block = 0;
	/*	
	while(nr_block <= 5){				
		lcd_gotoxy(nr_block,1);
		lcd_data((uint8_t)IconBlock);
		nr_block++;
		delay_ms(1000);
	}
	*/
	delay_ms(100);
	send_string0("AT+CREG?\r\n");
	delay_ms(100);
	send_string(getString0());
	
	lcd_gotoxy(nr_block,1);
	lcd_data((uint8_t)IconBlock);
	nr_block++;	
	
	delay_ms(2000);
	send_string0("AT+CMGF=1\r\n");
	delay_ms(100);
	send_string(getString0());	
	
	lcd_gotoxy(nr_block,1);
	lcd_data((uint8_t)IconBlock);
	nr_block++;	
	
	regexp_pattern_cmti = "+CMTI: \"[SME]{2}\",[0-9]{1,2}";
//+CMGR: "REC UNREAD\",\"[+]([0-9]{3,})
//+CMGR: "REC UNREAD","+4074480510","","17/11/06,19:26:50+08"
	regexp_pattern_call_number = "+CMGR: [\"]REC UNREAD[\",\"+]{4}[0-9]{11}";
	regexp_pattern_sms_ready = "SMS Ready";	
	
}


void Sim800l::searchRegexp(const char * pattern, const char * message_in){

	const TRexChar *begin,*end;
	
	TRexChar sTemp[100];
	const TRexChar *error = NULL;
	TRex *x = trex_compile(_TREXC(pattern),&error);
	
	if(x) {		
		trex_sprintf(sTemp,_TREXC(message_in));		
		if(trex_search(x, sTemp, &begin, &end))
		{
			int i,n = trex_getsubexpcount(x);
			TRexMatch match;
			
			trex_getsubexp(x, i, &match);			
			
			char target[50];
			* target = '\0';
			
			strncat(target, match.begin, match.len);			
			regexp_search_match = (const char *) target;
		}
		else{
			regexp_search_match = "No match\n";
		}
		trex_free(x);
	}
}
	

const char * Sim800l::checkSms(void){
  		
	const char * message_in = getString0();	
	//get_string_regexp(regexp_pattern_cmti, message_in);

	const char * result;
	searchRegexp(regexp_pattern_cmti, message_in);					// a regexp_search_match valtozoba teszi az eredmenyt
	//send_string(regexp_search_match);
		
	if(strncasecmp(regexp_search_match, "+CMTI:" , 6) == 0){
	
		send_string("Message Received\n");	
		send_string(message_in);
		char str_id[10];		
		uint16_t id;

		sms_id = strchr(regexp_search_match,',');
		sms_id++;	
		
		strcpy(msg_id_str, sms_id);								
				
		return sms_id;												
	}
	return "no sms";
}

void Sim800l::readSms(const char * sms_id){
		send_string("\n++++\n");
		send_string(sms_id);
		send_string("\n");
		deleteString0();
		deleteString();		
		send_string0("AT+CMGR=");
		send_string0(sms_id);
		send_string0("\r\n");	
		
		delay_ms(100);
		searchRegexp(regexp_pattern_call_number, getString0());						
		const char * call_nr;
		call_nr = strstr(regexp_search_match,"UNREAD\",\"");		
		call_nr += 9;		
		strcpy(call_number, call_nr);		

}

void Sim800l::SendSms(char * message){			
			send_string0("AT+CMGS=\"");
			send_string0(call_number);
			send_string0("\"\r\n");
			delay_ms(10);
			send_string0((const char *)message);
			delay_ms(10);
			send_string0("\x1A\r\n");						// CTRL+Z
			send_string("\nVALASZ ELKULDVE\n");
}


void Sim800l::DeleteAllSMS(void){	
	deleteString0();	
	send_string0("AT+CMGDA=DEL ALL\r\n");
	delay_ms(10);
	send_string(getString0());
}

char * Sim800l::GetNrOfSms(void){
	return msg_id_str;
}

void Sim800l::ResetNrOfSms(void){
	* msg_id_str = '\0';
}
