
#include "spi.h"

void spi_init (void){
	//configure spi pins alternate function and pull up on MISO PTD7
	PIN_MISO_PORT->PCR[PIN_MISO_BIT] = PORT_PCR_MUX(1)
																		 | PORT_PCR_PE(1)
																		 | PORT_PCR_PS(1);	//MISO PTD7
	PIN_MOSI_PORT->PCR[PIN_MOSI_BIT] = PORT_PCR_MUX(1);		//MOSI PTD6
	PIN_SCK_PORT->PCR[PIN_SCK_BIT]   = PORT_PCR_MUX(1); 	//SCK PTD5
	PIN_FL_RESET_PORT->PCR[PIN_FL_RESET_BIT] = PORT_PCR_MUX(1);		//FL_RESET PTB0
	PIN_FL_CS_PORT->PCR[PIN_FL_CS_BIT]       = PORT_PCR_MUX(1);		//FL_CS PTC2
	PIN_FL_W_PORT->PCR[PIN_FL_W_BIT]         = PORT_PCR_MUX(1);		//FL_W PTC1

	//configure spi pins direction
	PIN_MOSI_GPIO->PDDR     |= PIN_MOSI;
	PIN_SCK_GPIO->PDDR      |= PIN_SCK;
	PIN_FL_RESET_GPIO->PDDR |= PIN_FL_RESET;
	PIN_FL_CS_GPIO->PDDR    |= PIN_FL_CS;
	PIN_FL_W_GPIO->PDDR     |= PIN_FL_W;
	
	//set pins
	PIN_FL_RESET_GPIO->PSOR	= PIN_FL_RESET;
	PIN_FL_CS_GPIO->PSOR    = PIN_FL_CS;
	PIN_FL_W_GPIO->PSOR     = PIN_FL_W;
	
	PIN_SCK_GPIO->PCOR	= PIN_SCK;
	PIN_MOSI_GPIO->PCOR	= PIN_MOSI;
}

void spi_cs_low(void){
	PIN_FL_CS_GPIO->PCOR = PIN_FL_CS;
}

void spi_cs_high(void){
	PIN_FL_CS_GPIO->PSOR = PIN_FL_CS;
}

uint8_t spi_read(void){
	uint8_t data;
	spi_cs_low();
	data = spi_shift(0xFF);
	spi_cs_high();
	return data;
}

void spi_write(uint8_t val){
	spi_cs_low();
	spi_shift(val);
	spi_cs_high();
}

uint16_t spi_read_16(void){
	uint16_t data;
	spi_cs_low();
	data = spi_shift_16(0xFFFF);
	spi_cs_high();
	return data;
}

void spi_write_16(uint16_t val){
	spi_cs_low();
	spi_shift_16(val);
	spi_cs_high();
}

uint8_t spi_shift(uint8_t data){
	uint8_t val = 0;
	for( int i = 0 ; i < 8 ; i++ ){
		
		if( data & ( 0x80 >> i ) ){
			PIN_MOSI_GPIO->PSOR	= PIN_MOSI;
		}else{
			PIN_MOSI_GPIO->PCOR	= PIN_MOSI;
		}

		PIN_SCK_GPIO->PSOR	= PIN_SCK;val |= (((PIN_MISO_GPIO->PDIR & PIN_MISO) ? 1 : 0) << (7-i));
		PIN_SCK_GPIO->PCOR	= PIN_SCK;
		
	}
	return val;
}

uint16_t spi_shift_16(uint16_t data){
	uint16_t val = 0;
	for( int i = 0 ; i < 16 ; i++ ){
		
		if( data & ( 0x8000 >> i ) ){
			PIN_MOSI_GPIO->PSOR	= PIN_MOSI;
		}else{
			PIN_MOSI_GPIO->PCOR	= PIN_MOSI;
		}
		
		PIN_SCK_GPIO->PSOR	= PIN_SCK;val |= (((PIN_MISO_GPIO->PDIR & PIN_MISO) ? 1 : 0) << (15-i));
		PIN_SCK_GPIO->PCOR	= PIN_SCK;
		
	}
	return val;
}
