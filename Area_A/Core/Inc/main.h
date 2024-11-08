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
#define STM32_FLASH_START_ADDR        0x08000000
#define STM32_PAGE_SIZES              1024
#define STM32_PAGE_NUM                64
#define STM32_A_PAGE_NUM              20
#define STM32_B_PAGE_NUM              STM32_PAGE_NUM - STM32_A_PAGE_NUM
#define STM32_A_START_PAGE_NUM        STM32_A_PAGE_NUM
#define STM32_A_START__ADDR           STM32_FLASH_START_ADDR + STM32_A_START_PAGE_NUM * STM32_PAGE_SIZES
#define OTA_UPDATA_STATUS             0x1234
#define OTA_INFO_SIZE                 sizeof(OTA_Info)

typedef struct
{
    uint32_t ota_flag;
    
}OTA_Info;

extern OTA_Info OTA_Info_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
