#include <stdio.h>
#include "twi.h"
#include "system.h"
#include "delay.h"

unsigned int get_last_txt_address(void);
void set_last_txt_address(uint16_t block_addr);

void serial_eeprom_write_block(char data[]);

uint16_t serial_eeprom_read_text_start_address(void);
void serial_eeprom_write_text_start_address(uint16_t block_addr);

void writeEEPROM(unsigned int eeaddress, char* data);
const char *  readEEPROM(unsigned int eeaddress, unsigned int num_chars);

uint16_t get_last_txt_nr_pages(void);
void set_last_txt_nr_pages(uint16_t nr_page);