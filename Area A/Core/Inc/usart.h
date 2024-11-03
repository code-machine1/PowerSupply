/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
#define WIFI_RX_SIZE      2048
#define WIFI_TX_SIZE      2048
#define WIFI_RX_MAX       256
#define RX_DATA_BLOCK_NUM 10
typedef struct
{
    uint8_t *start;
    uint8_t *end;
}RX_BuffInfo_t;

typedef struct
{
    uint8_t wifi_rxbuff[WIFI_RX_SIZE];
    uint8_t wifi_txbuff[WIFI_TX_SIZE];
    uint8_t idle;
    uint16_t rx_counter;
    uint8_t rx_data;
    RX_BuffInfo_t rxdata_block[RX_DATA_BLOCK_NUM];
    RX_BuffInfo_t *rxdata_in;
    RX_BuffInfo_t *rxdata_out;
    RX_BuffInfo_t *rxdata_end;
}RX_DataInfo_t;

extern RX_DataInfo_t WIFI_RX_Data_t;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
extern void UART_Receive_IDLE(UART_HandleTypeDef *huart);
void UART2_RX_PtrInit(void);
void wifi_printf(char *format,...);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

