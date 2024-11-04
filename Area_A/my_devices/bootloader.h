#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H
#include "main.h"
#include "stm32f1xx.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "string.h"
#include "myflash.h"

typedef void (*load_a)(void);

void bootloader_judge(void);
__asm void MSR_SP(uint32_t addr);
void lOAD_A(uint32_t addr);
void bootloader_clear(void);

#endif
