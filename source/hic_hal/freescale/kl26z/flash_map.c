#include "flash_map.h"
#include "serial_flash.h"

//code
//INNFO BLOCK
void map_write_prog_entry(uint8_t prog_num, map_entry_t *entry){
	sf_write((uint8_t *)entry, MAP_INFO_ADDR_OF_PROG(prog_num), MAP_PROG_INFO_ENTRY_SIZE );
}

void map_read_prog_entry(uint8_t prog_num, map_entry_t *entry){
	sf_read((uint8_t *)entry, MAP_INFO_ADDR_OF_PROG(prog_num), MAP_PROG_INFO_ENTRY_SIZE);
}

//PROG BLOCK
#define MAP_PROG_BUFFER_SIZE			512		//buffer size for reading data from flash

void map_write_prog_data(uint8_t prog_num, uint32_t offset, uint8_t *data, uint32_t size){
	uint32_t address = MAP_DATA_ADDR_OF_PROG(prog_num) + offset;
	sf_write(data, address, size);
}
void map_read_prog_data(uint8_t prog_num, uint32_t offset, uint8_t *data, uint32_t size){
	uint32_t address = MAP_DATA_ADDR_OF_PROG(prog_num) + offset;
	sf_read(data,address, size);
}

void map_init(void){
	sf_init();
}
