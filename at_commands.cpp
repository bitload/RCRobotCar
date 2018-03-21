/*********************************************
ESP8266 control
*********************************************/

#include <avr/io.h>
#include "system.h"
#include "delay.h"


void send_command_to_esp(char command[], int delay)
{
	deleteString0();
	send_string0(command);	
	
	delay_ms(delay);
	
	//str_in = getString0();
	//send_string(str_in);		// send to usart 1
}
	
	