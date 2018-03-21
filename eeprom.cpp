#include <string.h>
#include "eeprom.h"		

		
#define     EXT_EEPROM_ADDR			0x50 // 24C32 EEPROM address	

char sms_read_data[60];
	
unsigned int get_last_txt_address(void){
	//write max block address used

	uint8_t lo_byte;
	uint8_t hi_byte;
	uint16_t max_address;
	
	twi_begin_transmission(EXT_EEPROM_ADDR);
	twi_send_byte(0x00);
	twi_send_byte(0x00);
	twi_end_transmission();	
	
	twi_request_from(EXT_EEPROM_ADDR, 2);	
	
	hi_byte = twi_receive();
	lo_byte = twi_receive();		
	twi_end_transmission();
	max_address = (hi_byte << 8) | lo_byte;
	
	return max_address;
	
}

void set_last_txt_address(uint16_t block_addr){
	
	uint8_t hi_byte;
	uint8_t lo_byte;		

	hi_byte = block_addr >> 8;
	lo_byte = block_addr & 0x00FF;
	
	twi_begin_transmission(EXT_EEPROM_ADDR);
	twi_send_byte(0x00);
	twi_send_byte(0x00);	
	twi_send_byte(hi_byte);			
	twi_send_byte(lo_byte);			
	twi_end_transmission();
	delay_ms(5);	
	
}

void serial_eeprom_write_block(char * data){	

	unsigned int write_addr = get_last_txt_address();					
	write_addr += 60;
	writeEEPROM(write_addr, data);		
	set_last_txt_address(write_addr);			
	
	send_string("BLOCK WRITE END\n");	
}


void writeEEPROM(unsigned int eeaddress, char * data)
{
	// Uses Page Write for 24LC256
	// Allows for 64 byte page boundary
	// Splits string into max 16 byte writes
	unsigned char i=0, counter=0;
	unsigned int  address;
	unsigned int  page_space;
	unsigned int  page=0;
	unsigned int  num_writes;
	unsigned int  data_len=0;
	unsigned char first_write_size;
	unsigned char last_write_size;
	unsigned char write_size;
	
	// Calculate length of data
	do{ data_len++; } while(data[data_len]);
	
	// Calculate space available in first page
	page_space = int(((eeaddress/64) + 1)*64)-eeaddress;

	// Calculate first write size
	if (page_space>16){
		first_write_size=page_space-((page_space/16)*16);
		if (first_write_size==0) first_write_size=16;
	}
	else
	first_write_size=page_space;
	
	// calculate size of last write
	if (data_len>first_write_size)
	last_write_size = (data_len-first_write_size)%16;
	
	// Calculate how many writes we need
	if (data_len>first_write_size)
	num_writes = ((data_len-first_write_size)/16)+2;
	else
	num_writes = 1;
	
	i=0;
	address=eeaddress;
		
	for(page=0;page<num_writes;page++)
	{
		if(page==0) write_size=first_write_size;
		else if(page==(num_writes-1)) write_size=last_write_size;
		else write_size=16;
		
		twi_begin_transmission(EXT_EEPROM_ADDR);
		twi_send_byte((int)((address) >> 8));   // MSB
		twi_send_byte((int)((address) & 0xFF)); // LSB
		counter=0;
		do{
			twi_send_byte((uint8_t) data[i]);
			i++;
			counter++;
		} while((data[i]) && (counter<write_size));
		twi_end_transmission();
		address+=write_size;   // Increment address for next write
		
		delay_ms(6);  // needs 5ms for page write
	}
}

const char * readEEPROM(unsigned int eeaddress, unsigned int num_chars)
{
	unsigned char i=0;
	twi_begin_transmission(EXT_EEPROM_ADDR);
	twi_send_byte((int)(eeaddress >> 8));   // MSB
	twi_send_byte((int)(eeaddress & 0xFF)); // LSB
	twi_end_transmission();
	
	twi_request_from(EXT_EEPROM_ADDR, num_chars);
	
	while(twi_available()) sms_read_data[i++] = twi_receive();
	return sms_read_data;
	//while(twi_available()) SendCharUart1(twi_receive());

}