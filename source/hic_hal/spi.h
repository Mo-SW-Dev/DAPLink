#ifndef SPI_H
#define SPI_H

#include "IO_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

//Protos

void spi_init (void);
void spi_cs_low(void);
void spi_cs_high(void);
uint8_t spi_shift(uint8_t data);
uint16_t spi_shift_16(uint16_t data);
uint8_t spi_read(void);
uint16_t spi_read_16(void);
void spi_write(uint8_t val);
void spi_write_16(uint16_t val);

#ifdef __cplusplus
}
#endif

#endif
