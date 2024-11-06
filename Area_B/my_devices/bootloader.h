#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H
#include "main.h"
#include "stm32f1xx.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "string.h"
#include "myflash.h"
#include "store.h"
#include "w25qxx.h"
typedef void (*load_a)(void);
uint32_t readota_flag(void);
uint32_t writeota_flag(uint32_t flag);
void bootloader_judge(void);
__asm void MSR_SP(uint32_t addr);
void lOAD_A(uint32_t addr);
void bootloader_clear(void);
uint8_t bootloader_enter(uint8_t timeout);
void bootloader_info(void);
void bootloader_event(uint8_t *data,uint16_t datalen);
uint16_t bootloader_crc16(uint8_t * data,uint16_t datalen);
void write_otainfo(void);
void read_otainfo(void);
#endif
