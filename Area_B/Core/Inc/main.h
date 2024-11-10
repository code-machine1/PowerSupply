/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define WIFI_RST_Pin GPIO_PIN_1
#define WIFI_RST_GPIO_Port GPIOA
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define TFT_SCL_Pin GPIO_PIN_3
#define TFT_SCL_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_4
#define TFT_DC_GPIO_Port GPIOB
#define TFT_SDA_Pin GPIO_PIN_5
#define TFT_SDA_GPIO_Port GPIOB
#define TFT_RES_Pin GPIO_PIN_6
#define TFT_RES_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_7
#define TFT_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define STM32_FLASH_START_ADDR        0x08000000          //stm32程序起始地址
#define STM32_PAGE_SIZES              1024                //stm32内部flash一页的字节数
#define STM32_PAGE_NUM                64                  //stm32内部flash的总页数
#define STM32_A_PAGE_NUM              36                  //A区的页数
#define STM32_B_PAGE_NUM              STM32_PAGE_NUM - STM32_A_PAGE_NUM   //B区的页数 
#define STM32_A_START_PAGE_NUM        STM32_A_PAGE_NUM    //A区的起始页数
#define STM32_A_START_ADDR            STM32_FLASH_START_ADDR + STM32_A_START_PAGE_NUM * STM32_PAGE_SIZES  //A区的起始地址
#define OTA_UPDATA_STATUS             0x1234              //OTA升级标志位
#define OTA_INFO_SIZE                 sizeof(OTA_Info)    //计算要保存在B区结构体的数据大小
#define OTA_INFO_ADDR                 0x08006C00          //OTA数据保存的起始地址
#define UPDATA_A_FLAG                 0x00000001          //A区升级标志位
#define IAP_XMODEM_FLAG               0X00000002          //Xmodem传输方式标志位
#define IAP_XMODEMDATA_FLAG           0X00000004          //Xmodem传输开始标志位
#define SET_VERSION_FLAG              0X00000008          //设置版本信息标志位
#define CMD_5_FLAG                    0X00000010          //命令5传输标志位（和Xmodem没有区别）
#define CMD_5_XMODEM_FLAG             0X00000020          //命令5开始标志位（和Xmodem没有区别）
#define CMD_6_FLAG                    0X00000040          //从外部下载程序标志位

//VER-1.0.0-2024/11/5-11:00

typedef struct
{
    uint32_t ota_flag;        //存放OTA升级标志位
    uint32_t firelen[11];     //存放当前程序的字节大小信息
    uint32_t ota_version[32]; //存放OTA版本信息
}OTA_Info;  //这些数据保存在B区的最后1K flash里面，掉电不丢失。

typedef struct
{
    uint8_t Updatabuff[STM32_PAGE_SIZES];    //升级数据的缓存数组
    uint32_t W25Q32_BlockNumber;             //W25Q32块信息的情况
    uint8_t Xmodemtime;                      //Xmodem协议帧开头计时（1秒发送一次字符C给上位机表示准备接收程序）
    uint32_t Xmodemnumber;                   //Xmodem协议接收数据包个数计算
    uint32_t Xmodemnumcrc;                   //Xmodem协议要用到crc校验
}UpData_Info;  //这些数据都是记录一些升级情况用的（收了多少包、存到了哪个块中等等）

extern UpData_Info UpData_Info_t;
extern OTA_Info OTA_Info_t;
extern uint32_t boot_startflag;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
