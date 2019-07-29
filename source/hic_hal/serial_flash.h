#ifndef SERIAL_FLASH_H
#define SERIAL_FLASH_H

#include "spi.h"

#ifdef __cplusplus
extern "C" {
#endif

//status register cmd 0x05
#define SF_WIP_MASK  1
#define SF_WIP_BIT	 0

#define SF_WEL_MASK  2
#define SF_WEL_BIT   1

#define SF_BP_MASK   (15<<2)
#define SF_BP_BIT    2

#define SF_QE_MASK   (1<<6)
#define SF_QE_BIT    6

#define SF_SRWD_MAKS (1<<7)
#define SF_SRWD_BIT  7

//bank address register cmd 0x??
#define SF_EXTADD_MASK (1<<7)
#define SF_EXTADD_BIT  7

//function register
#define SF_ESUS_MAKS (1<<3)
#define SF_ESUS_BIT  3

#define SF_PSUS_MAKS (1<<2)
#define SF_PSUS_BIT  2

//Protos

void sf_init(void);
void sf_read(uint8_t *buf, uint32_t add, uint32_t len);
void sf_write(uint8_t *buf, uint32_t add, uint32_t len);
uint8_t sf_sfdp(uint8_t add);

uint8_t sf_status(void);      //05h
uint8_t sf_bank_reg(void); //16h/C8h
void sf_is_busy(void);
void sf_delete_sector(uint32_t sec_add);
void sf_delete_block(uint32_t blk_add);
void sf_delete_chip(void);
void sf_write_enable(void);
void sf_suspend(void);
void sf_resume(void);
uint8_t sf_func_reg(void);
#ifdef __cplusplus
}
#endif

#endif
