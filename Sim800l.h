#ifndef Sim800l_h
#define Sim800l_h
#include <stddef.h>
#include <avr/pgmspace.h>

#define STR_STATE_ON_AUTO			"Auto"
#define STR_STATE_ON_MAUAL			"Manual"
#define STR_STATE_OFF				"Off"
#define STR_STATE_IDLE				"Idle"
#define STR_TEMP_MAN_MODIF			"Beallit"
#define NR_MESSAGES_STORE			40
#define SMS_TEMP_QUERY				"Temp"
#define NO_CREDIT_MESSAGE_REC		""

//RDY
//+CFUN: 1
//+CPIN: READY
//Call Ready
//SMS Ready
#define STR_SMS_READY "SMS Ready"

//Ha nincs eleg credit:
//Ne pare rau dar nu ai suficient credit pentru a trimite SMS catre acest numar. Te rugam sa-ti reincarci contul.

// Hany SMS van:
//AT+CPMS="ME"
//+CPMS: 0,50,10,10,10,10
//+CPMS: 1,50,10,10,10,10
//+CPMS: ...,50,10,10,10,10

class Sim800l		
{									
  private:
	const char * message_in;
	const char * message_out;	
	const char * regexp_pattern_cmti;
		//+CMGR: "REC UNREAD\",\"[+]([0-9]{3,})
		//+CMGR: "REC UNREAD","+4074480510","","17/11/06,19:26:50+08"
	const char * regexp_pattern_call_number;
	const char * regexp_pattern_sms_ready;
	char call_number[20];	
	

  public:    	
	const char * checkSms(void);
	void readSms(const char * sms_id);
	void searchRegexp(const char * pattern, const char * message_in);
	void InitSIM(void);
	const char * regexp_search_match;	
	void SendSms(char * message);
	void DeleteAllSMS(void);
	char * GetNrOfSms(void);	
	void ResetNrOfSms(void);
	char * sms_id;
	char msg_id_str[3];

};

#endif 