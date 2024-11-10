/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include "w25qxx.h"
#include "bootloader.h"
#include "myflash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */



OTA_Info OTA_Info_t;

UpData_Info UpData_Info_t;

uint32_t boot_startflag;
uint32_t i;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_SPI2_Init();
    /* USER CODE BEGIN 2 */
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart2,WIFI_RX_Data_t.rxdata_in->start,WIFI_RX_MAX);
    BSP_W25Qx_Init();
//    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
    
    bootloader_judge();//判断是否跳转到A区
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        HAL_Delay(10);
        if(WIFI_RX_Data_t.rxdata_out != WIFI_RX_Data_t.rxdata_in)//串口接收的数据记录。（因为是空闲中断，数据长度是不定的，且开启了DMA搬运功能，需要将一段数据搬运到数组里，并记录这段数据的开头地址和结束地址，参考队列的实现方式）
        {
            bootloader_event(WIFI_RX_Data_t.rxdata_out->start,WIFI_RX_Data_t.rxdata_out->end - WIFI_RX_Data_t.rxdata_out->start +1);//把数据段的起始地址和地址的长度传进去
            WIFI_RX_Data_t.rxdata_out++;
            if(WIFI_RX_Data_t.rxdata_out == WIFI_RX_Data_t.rxdata_end)//这个队列到头了的话
            {
                WIFI_RX_Data_t.rxdata_out = &WIFI_RX_Data_t.rxdata_block[0];//重修指到开头，防止数组溢出
            }

        }

        if(boot_startflag & IAP_XMODEM_FLAG)//开始Xmodem传输（这个标志位在bootloader_event函数里面被置位或者清零）
        {
            if(UpData_Info_t.Xmodemtime>=100)//搭配整个系统10ms的延时，这里进来就是1秒
            {
                wifi_printf("C");        //每秒钟发一个字符C告诉上位机准备OK，等待接收程序
                UpData_Info_t.Xmodemtime = 0;//计时的标量
            }
            UpData_Info_t.Xmodemtime++;
        }


        if(boot_startflag & UPDATA_A_FLAG)  //开始A区更新（这个标志位在bootloader_event函数里面被置位或者清零）
        {
            wifi_printf("长度%d字节\r\n",OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber]); 
            if(0 == OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] % 4) //检查数据包长度
            {
                stm32_eraseflash(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM); //写入A区之前先擦除A区
                for(i=0; i<OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber]/STM32_PAGE_SIZES; i++)//先分解数据包 算出来有多少k 再按照每次1k的量来写入
                {
                    BSP_W25Qx_Read_Data(UpData_Info_t.Updatabuff,i*1024+UpData_Info_t.W25Q32_BlockNumber*64*1024,STM32_PAGE_SIZES);  //直接从外部flash面先读到缓存数组
                    stm32_writeflash(STM32_A_START_ADDR + i * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,STM32_PAGE_SIZES);//然后写入到单片机
                }
                if(0 != OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] % 1024 )// 如果最后的数据不足1k了，就不需要便宜1k的地址，直接写入就好了
                {
                    BSP_W25Qx_Read_Data(UpData_Info_t.Updatabuff,i*1024+UpData_Info_t.W25Q32_BlockNumber*64*1024,OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] % 1024);//直接从外部flash面先读到缓存数组
                    stm32_writeflash(STM32_A_START_ADDR + i * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] % 1024);//然后写入到单片机
                }
                if(0 == OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber])//等到缓存数组里的数据下完了就把标志位清掉
                {
                    OTA_Info_t.ota_flag=0;
                    write_otainfo();//保存当前版本信息
                    boot_startflag &=~ UPDATA_A_FLAG;
                }
                wifi_printf("A区更新完毕\r\n");
                NVIC_SystemReset();//重启单片机
            }
            else
            {
                wifi_printf("长度错误\r\n");
                boot_startflag &=~ UPDATA_A_FLAG;
            }

        }


        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
