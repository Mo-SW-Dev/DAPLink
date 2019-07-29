
#include "serial_flash.h"
#include "string.h"

void sf_init(void){
	spi_init();
	PIN_FL_RESET_GPIO->PCOR	= PIN_FL_RESET;
	PIN_FL_RESET_GPIO->PSOR	= PIN_FL_RESET;
}

void sf_read(uint8_t *buf, uint32_t addr, uint32_t len){
	
	uint8_t suspend = 0;
	if(sf_status() & SF_WIP_MASK){//suspend if program/erase is in progress
		sf_suspend();
		suspend = 1;
	}
	
	uint8_t *p = (uint8_t *)buf;

	sf_is_busy();		//wait while busy
	
	do {
		uint32_t rdlen = len;
		uint8_t reg = sf_bank_reg();
		spi_cs_low();
		if ( reg & SF_EXTADD_MASK) {
			spi_shift(0x03);
			spi_shift_16(addr >> 16);
			spi_shift_16(addr);
		} else {
			spi_shift_16(0x0300 | ((addr >> 16) & 255));
			spi_shift_16(addr);
		}
		for(int i = 0; i<rdlen; i++){
			p[i] = spi_shift(0xFF);
		}
		spi_cs_high();
		p += rdlen;
		addr += rdlen;
		len -= rdlen;
	} while (len > 0);
	
	if(suspend){
		sf_resume();
	}
}

void sf_write(uint8_t *buf, uint32_t addr, uint32_t len){
	
	uint8_t suspend = 0;
	if(sf_func_reg() & SF_PSUS_MAKS){//wait if program is in progress
		sf_is_busy();
	}else{//suspend if erase is in progress
		sf_suspend();
		suspend = 1;
	}
	
	const uint8_t *p = (const uint8_t *)buf;
	uint32_t max, pagelen;

	 //Serial.printf("WR: addr %08X, len %d\n", addr, len);
	do {
		sf_is_busy();
		
		sf_write_enable();

		max = 256 - (addr & 0xFF);
		pagelen = (len <= max) ? len : max;
		
		uint8_t reg = sf_bank_reg();
		spi_cs_low();
		if ( reg & SF_EXTADD_MASK) {
			spi_shift(0x02); // program page command
			spi_shift_16(addr >> 16);
			spi_shift_16(addr);
		} else {
			spi_shift_16(0x0200 | ((addr >> 16) & 255));
			spi_shift_16(addr);
		}
		addr += pagelen;
		len -= pagelen;
		do {
			spi_shift(*p++);
		} while (--pagelen > 0);
		spi_cs_high();
	} while (len > 0);
	
	if(suspend){
		sf_resume();
	}
}

uint8_t sf_sfdp(uint8_t add){
	uint8_t data;
	spi_cs_low();
	spi_shift(0x5a);
	spi_shift(0x00);
	spi_shift(0x00);
	spi_shift(add);
	spi_shift(0xFF);	//dummy
	data = spi_shift(0xFF);
	spi_cs_high();
	return data;
}

void sf_is_busy(void){
	uint8_t status;
	while(1){
		status = sf_status();
		if(!(status & SF_WIP_MASK)){
			break;
		}
	}
}

uint8_t sf_status(void){
	uint8_t reg;
	spi_cs_low();
	spi_shift(0x05);
	reg = spi_shift(0xFF);
	spi_cs_high();
	return reg;
}

uint8_t sf_bank_reg(void){
	uint8_t reg;
	spi_cs_low();
	spi_shift(0x16);
	reg = spi_shift(0xFF);
	spi_cs_high();
	return reg;
}


void sf_delete_chip(void){
	sf_is_busy();
	
	sf_write_enable();
	
	//chip erase  C7h/60h
	spi_cs_low();
	spi_shift(0xC7);
	spi_cs_high();
}

void sf_delete_block(uint32_t blk_add){
	uint8_t suspend = 0;
	if(sf_func_reg() & SF_ESUS_MAKS){//wait if erase is in progress
		sf_is_busy();
	}else{//suspend if programming is in progress
		sf_suspend();
		suspend = 1;
	}
	
	sf_write_enable();
	
	spi_cs_low();
	//block erase
	//D8h=64k 52h=32k
	if (sf_bank_reg() & SF_EXTADD_MASK) {
		spi_shift(0x52);
		spi_shift_16(blk_add >> 16);
		spi_shift_16(blk_add);
	} else {
		spi_shift_16(0x5200 | ((blk_add >> 16) & 255));
		spi_shift_16(blk_add);
	}
	spi_cs_high();
	
	if(suspend){
		sf_resume();
	}
}

void sf_delete_sector(uint32_t sec_add){
	
	uint8_t suspend = 0;
	if(sf_func_reg() & SF_ESUS_MAKS){//wait if erase is in progress
		sf_is_busy();
	}else{//suspend if programming is in progress
		sf_suspend();
		suspend = 1;
	}
	
	sf_write_enable();

	spi_cs_low();
	//sector erase D7h/20h
	if (sf_bank_reg() & SF_EXTADD_MASK) {
		spi_shift(0x20);
		spi_shift_16(sec_add >> 16);
		spi_shift_16(sec_add);
	} else {
		spi_shift_16(0x2000 | ((sec_add >> 16) & 255));
		spi_shift_16(sec_add);
	}
	spi_cs_high();
	
	if(suspend){
		sf_resume();
	}
}

void sf_write_enable(void){
	spi_cs_low();
	// write enable command
	spi_shift(0x06);
	spi_cs_high();
	
	uint8_t status;
	while(1){
		status = sf_status();
		if(status & SF_WEL_MASK){
			break;
		}
	}
}

void sf_suspend(void){
	//75h/B0h
	spi_write(0x75);
}

void sf_resume(void){
	//7Ah/30h
	spi_write(0x7A);
}

uint8_t sf_func_reg(void){
	uint8_t reg;
	spi_cs_low();
	spi_shift(0x48);
	reg = spi_shift(0xFF);
	spi_cs_high();
	return reg;
}

