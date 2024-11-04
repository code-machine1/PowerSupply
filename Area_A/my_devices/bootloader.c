#include "bootloader.h"
extern DMA_HandleTypeDef hdma_usart2_rx;
load_a load_A;

void bootloader_judge(void)
{
    OTA_Info_t.ota_flag = 0;
    OTA_Info_t.ota_flag = MyFLASH_ReadHalfWord(0x0800FC02);
    if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)
        wifi_printf("OTA更新\r\n");
    else
    {
        wifi_printf("跳转系统分区（A区）\r\n");
        lOAD_A(STM32_A_START__ADDR);
    }
        
}

__asm void MSR_SP(uint32_t addr)
{
    MSR MSP, r0
    BX r14
}

void lOAD_A(uint32_t addr)
{
  if((*(uint32_t *)addr >= 0x20000000) && (*(uint32_t *)addr <=  0x20004fff))
  {
      MSR_SP(*(uint32_t *)addr);
      load_A = (load_a)*(uint32_t *)(addr+4);
      load_A();
  }

}

void bootloader_clear(void)
{
    HAL_GPIO_DeInit(GPIOA,GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB,GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC,GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOD,GPIO_PIN_All);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_SPI_DeInit(&hspi2);
    HAL_DMA_DeInit(&hdma_usart2_rx);
}
