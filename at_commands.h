#ifndef __ATCOMMANDS
#define __ATCOMMANDS

#define		AT				"AT\r\n"
#define		AT_CWJAP		"AT+CWJAP="
#define		AT_CIPMUX		"AT+CIPMUX=1\r\n"
#define		AT_CIPSERVER	"AT+CIPSERVER=1,333\r\n"
#define		AT_CIFSR		"AT+CIFSR\r\n"
#define		AT_RST			"AT+RST\r\n"
#define		AT_CWJAP_Q		"AT+CWJAP?\r\n"
#define		AT_CWQAP		"AT+CWQAP\r\n"

void send_command_to_esp(char * command, int delay);

#endif