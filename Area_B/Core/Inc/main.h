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
#define STM32_FLASH_START_ADDR        0x08000000          //stm32������ʼ��ַ
#define STM32_PAGE_SIZES              1024                //stm32�ڲ�flashһҳ���ֽ���
#define STM32_PAGE_NUM                64                  //stm32�ڲ�flash����ҳ��
#define STM32_A_PAGE_NUM              36                  //A����ҳ��
#define STM32_B_PAGE_NUM              STM32_PAGE_NUM - STM32_A_PAGE_NUM   //B����ҳ�� 
#define STM32_A_START_PAGE_NUM        STM32_A_PAGE_NUM    //A������ʼҳ��
#define STM32_A_START_ADDR            STM32_FLASH_START_ADDR + STM32_A_START_PAGE_NUM * STM32_PAGE_SIZES  //A������ʼ��ַ
#define OTA_UPDATA_STATUS             0x1234              //OTA������־λ
#define OTA_INFO_SIZE                 sizeof(OTA_Info)    //����Ҫ������B���ṹ������ݴ�С
#define OTA_INFO_ADDR                 0x08006C00          //OTA���ݱ������ʼ��ַ
#define UPDATA_A_FLAG                 0x00000001          //A��������־λ
#define IAP_XMODEM_FLAG               0X00000002          //Xmodem���䷽ʽ��־λ
#define IAP_XMODEMDATA_FLAG           0X00000004          //Xmodem���俪ʼ��־λ
#define SET_VERSION_FLAG              0X00000008          //���ð汾��Ϣ��־λ
#define CMD_5_FLAG                    0X00000010          //����5�����־λ����Xmodemû������
#define CMD_5_XMODEM_FLAG             0X00000020          //����5��ʼ��־λ����Xmodemû������
#define CMD_6_FLAG                    0X00000040          //���ⲿ���س����־λ

//VER-1.0.0-2024/11/5-11:00

typedef struct
{
    uint32_t ota_flag;        //���OTA������־λ
    uint32_t firelen[11];     //��ŵ�ǰ������ֽڴ�С��Ϣ
    uint32_t ota_version[32]; //���OTA�汾��Ϣ
}OTA_Info;  //��Щ���ݱ�����B�������1K flash���棬���粻��ʧ��

typedef struct
{
    uint8_t Updatabuff[STM32_PAGE_SIZES];    //�������ݵĻ�������
    uint32_t W25Q32_BlockNumber;             //W25Q32����Ϣ�����
    uint8_t Xmodemtime;                      //XmodemЭ��֡��ͷ��ʱ��1�뷢��һ���ַ�C����λ����ʾ׼�����ճ���
    uint32_t Xmodemnumber;                   //XmodemЭ��������ݰ���������
    uint32_t Xmodemnumcrc;                   //XmodemЭ��Ҫ�õ�crcУ��
}UpData_Info;  //��Щ���ݶ��Ǽ�¼һЩ��������õģ����˶��ٰ����浽���ĸ����еȵȣ�

extern UpData_Info UpData_Info_t;
extern OTA_Info OTA_Info_t;
extern uint32_t boot_startflag;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
