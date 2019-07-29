#ifndef FLASH_MAP_H
#define FLASH_MAP_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

//INFO BLOCK
/*
 * mem
 * prog_num
 *				|
 *				V  __START_____END______
 *				0 |_________|___________|
 *				1 |_________|___________|
 *				. |_________|___________|
 * 				. |_________|___________|
 *
*/

#define MAP_PROG_INFO_START_ADDR		0			//the starting address of the info block in mem
#define MAP_PROG_MAX								26		//max alllowed program count
#define MAP_PROG_INFO_ENTRY_SIZE		8			// 8 bytes in size

#define MAP_INFO_ADDR_OF_PROG(PROG_NUMBER) (MAP_PROG_INFO_START_ADDR+(PROG_NUMBER*MAP_PROG_INFO_ENTRY_SIZE)) //get program info n address in mem

typedef struct{
	uint32_t start;
	uint32_t end;
}map_entry_t;

//protos
void map_write_prog_entry(uint8_t prog_num, map_entry_t *entry);
void map_read_prog_entry(uint8_t prog_num, map_entry_t *entry);

//PROG BLOCK

#define MAP_PROG_BUFFER_SIZE				512				//buffer size for reading data from flash
#define MAP_PROG_MAX_SIZE						500000UL	//maximum program size allowed
#define MAP_PROG_DATA_START_ADDR		(MAP_PROG_INFO_START_ADDR + ((MAP_PROG_MAX + 1 ) * MAP_PROG_INFO_ENTRY_SIZE))	//start address of the program block
#define MAP_DATA_ADDR_OF_PROG(PROG_NUMBER) (MAP_PROG_DATA_START_ADDR + (PROG_NUMBER * MAP_PROG_MAX_SIZE))								//get program n adress in mem

void map_write_prog_data(uint8_t prog_num, uint32_t offset, uint8_t *data, uint32_t size);
void map_read_prog_data(uint8_t prog_num, uint32_t offset, uint8_t *data, uint32_t size);

void map_init(void);

#ifdef __cplusplus
}
#endif

#endif
