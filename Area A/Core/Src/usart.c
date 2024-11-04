/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"

//uint8_t wifi_rxbuff[WIFI_RX_SIZE];
RX_DataInfo_t WIFI_RX_Data_t;

int fputc(int ch,FILE *f)
{
    HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY );
    return ch;
}

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 921600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */
    UART2_RX_PtrInit();
    /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==USART1)
    {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
    else if(uartHandle->Instance==USART2)
    {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 DMA Init */
        /* USART2_RX Init */
        hdma_usart2_rx.Instance = DMA1_Channel6;
        hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_rx.Init.Mode = DMA_NORMAL;
        hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

    if(uartHandle->Instance==USART1)
    {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

        /* USART1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
    else if(uartHandle->Instance==USART2)
    {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

        /* USART2 DMA DeInit */
        HAL_DMA_DeInit(uartHandle->hdmarx);

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

void UART_Receive_IDLE(UART_HandleTypeDef *huart)
{
    uint32_t temp;
    if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        HAL_UART_DMAStop(huart);
        temp = huart->hdmarx->Instance->CNDTR;

        if(huart->Instance == huart2.Instance)
        {
            WIFI_RX_Data_t.rx_counter += WIFI_RX_MAX - temp;
            WIFI_RX_Data_t.rxdata_in->end = &WIFI_RX_Data_t.wifi_rxbuff[WIFI_RX_Data_t.rx_counter -1];
            WIFI_RX_Data_t.rxdata_in++;
            if(WIFI_RX_Data_t.rxdata_in == WIFI_RX_Data_t.rxdata_end)
            {
                WIFI_RX_Data_t.rxdata_in = &WIFI_RX_Data_t.rxdata_block[0];
            }
            if(WIFI_RX_SIZE - WIFI_RX_Data_t.rx_counter >= WIFI_RX_MAX)
            {
                WIFI_RX_Data_t.rxdata_in->start = &WIFI_RX_Data_t.wifi_rxbuff[WIFI_RX_Data_t.rx_counter];
            }
            else
            {
                WIFI_RX_Data_t.rxdata_in->start = WIFI_RX_Data_t.wifi_rxbuff;
                WIFI_RX_Data_t.rx_counter = 0;
            }

            WIFI_RX_Data_t.idle = 1;
            HAL_UART_Receive_DMA(&huart2,WIFI_RX_Data_t.rxdata_in->start,WIFI_RX_MAX);
        }
    }
}


void UART2_RX_PtrInit(void)
{
    WIFI_RX_Data_t.rxdata_in = &WIFI_RX_Data_t.rxdata_block[0];
    WIFI_RX_Data_t.rxdata_out = &WIFI_RX_Data_t.rxdata_block[0];
    WIFI_RX_Data_t.rxdata_end = &WIFI_RX_Data_t.rxdata_block[RX_DATA_BLOCK_NUM-1];
    WIFI_RX_Data_t.rx_counter = 0;
    WIFI_RX_Data_t.rxdata_in->start = WIFI_RX_Data_t.wifi_rxbuff;
}

void wifi_printf(char *format,...)
{
    uint16_t i;
    va_list listdata;
    va_start(listdata,format);
    vsprintf((char *)WIFI_RX_Data_t.wifi_txbuff,format,listdata);
    va_end(listdata);
    for(i=0; i<strlen((char *)WIFI_RX_Data_t.wifi_txbuff); i++)
    {
        while(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TXE) != 1)
            ;
        HAL_UART_Transmit(&huart2,&WIFI_RX_Data_t.wifi_txbuff[i],1,HAL_MAX_DELAY);
    }
    while(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) != 1)
        ;
}



/* USER CODE END 1 */
