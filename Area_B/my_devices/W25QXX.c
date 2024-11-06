#include "W25Qxx.h"

/**********************************************************************************
 * ��������: ģ���ʼ��
 */
uint8_t BSP_W25Qx_Init(void)
{
    BSP_W25Qx_Reset();
    return BSP_W25Qx_GetStatus();
}


static void BSP_W25Qx_Reset(void)
{
    uint8_t cmd[2] = {RESET_ENABLE_CMD,RESET_MEMORY_CMD};

    W25Qx_Enable();
    /* Send the reset command */
    HAL_SPI_Transmit(&hspi2, cmd, 2, W25Qx_TIMEOUT_VALUE);
    W25Qx_Disable();

}

/**********************************************************************************
 * ��������: ��ȡ�豸״̬
 */
static uint8_t BSP_W25Qx_GetStatus(void)
{
    uint8_t cmd[] = {READ_STATUS_REG1_CMD};
    uint8_t status;

    W25Qx_Enable();
    /* Send the read status command */
    HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);
    /* Reception of the data */
    HAL_SPI_Receive(&hspi2,&status, 1, W25Qx_TIMEOUT_VALUE);
    W25Qx_Disable();

    /* Check the value of the register */
    if((status & W25Q128FV_FSR_BUSY) != 0)
    {
        return W25Qx_BUSY;
    }
    else
    {
        return W25Qx_OK;
    }
}

/**********************************************************************************
 * ��������: дʹ��
 */
uint8_t BSP_W25Qx_WriteEnable(void)
{
    uint8_t cmd[] = {WRITE_ENABLE_CMD};
    uint32_t tickstart = HAL_GetTick();

    /*Select the FLASH: Chip Select low */
    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);
    /*Deselect the FLASH: Chip Select high */
    W25Qx_Disable();

    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
        {
            return W25Qx_TIMEOUT;
        }
    }

    return W25Qx_OK;
}

/**********************************************************************************
 * ��������: ��ȡ�豸ID
 */
void BSP_W25Qx_Read_ID(uint8_t *ID)
{
    uint8_t cmd[4] = {READ_ID_CMD,0x00,0x00,0x00};

    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);
    /* Reception of the data */
    HAL_SPI_Receive(&hspi2,ID, 2, W25Qx_TIMEOUT_VALUE);
    W25Qx_Disable();

}

/**********************************************************************************
 * ��������: ������
 * �������: ��������ָ�롢����ַ���ֽ���
 */
uint8_t BSP_W25Qx_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
    uint8_t cmd[4];

    /* Configure the command */
    cmd[0] = READ_CMD;
    cmd[1] = (uint8_t)(ReadAddr >> 16);
    cmd[2] = (uint8_t)(ReadAddr >> 8);
    cmd[3] = (uint8_t)(ReadAddr);

    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);
    /* Reception of the data */
    if (HAL_SPI_Receive(&hspi2, pData,Size,W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
        return W25Qx_ERROR;
    }
    W25Qx_Disable();
    return W25Qx_OK;
}



void BSP_W25Qx_Write_Blocks(uint32_t start_addr ,uint32_t *wdata,uint32_t num)
{
    while(num)
    {
        MyFLASH_ProgramWord(start_addr,*wdata);
        //BSP_W25Qx_Write((uint8_t *)wdata,start_addr,1);
        num-=4;
        start_addr+=4;
        wdata++;
    }


}



uint8_t BSP_W25Qx_Page_Write(uint8_t* pData, uint16_t pagenumber)
{
    uint32_t tickstart = HAL_GetTick();
    uint8_t wdata[4];
    wdata[0] = 0x20;
    wdata[1] = (pagenumber*256)>>16;
    wdata[2] = (pagenumber*256)>>8;
    wdata[3] = (pagenumber*256)>>0;
    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
        {
            return W25Qx_TIMEOUT;
        }
    }

    BSP_W25Qx_WriteEnable();

    /*Select the FLASH: Chip Select low */
    W25Qx_Enable();

    /* Send the command */
    if (HAL_SPI_Transmit(&hspi2,wdata, 4, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
        return W25Qx_ERROR;
    }

    /* Transmission of the data */
    if (HAL_SPI_Transmit(&hspi2, pData,256, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
        return W25Qx_ERROR;
    }
    W25Qx_Disable();
    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
        {
            return W25Qx_TIMEOUT;
        }
    }
    return 0;
}







/**********************************************************************************
 * ��������: д����
 * �������: ��������ָ�롢д��ַ���ֽ���
 */
uint8_t BSP_W25Qx_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
    uint8_t cmd[4];
    uint32_t end_addr, current_size, current_addr;
    uint32_t tickstart = HAL_GetTick();

    /* Calculation of the size between the write address and the end of the page */
    current_addr = 0;

    while (current_addr <= WriteAddr)
    {
        current_addr += W25Q128FV_PAGE_SIZE;
    }
    current_size = current_addr - WriteAddr;

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
        current_size = Size;
    }

    /* Initialize the adress variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;

    /* Perform the write page by page */
    do
    {
        /* Configure the command */
        cmd[0] = PAGE_PROG_CMD;
        cmd[1] = (uint8_t)(current_addr >> 16);
        cmd[2] = (uint8_t)(current_addr >> 8);
        cmd[3] = (uint8_t)(current_addr);

        /* Enable write operations */
        BSP_W25Qx_WriteEnable();

        W25Qx_Enable();
        /* Send the command */
        if (HAL_SPI_Transmit(&hspi2,cmd, 4, W25Qx_TIMEOUT_VALUE) != HAL_OK)
        {
            return W25Qx_ERROR;
        }

        /* Transmission of the data */
        if (HAL_SPI_Transmit(&hspi2, pData,current_size, W25Qx_TIMEOUT_VALUE) != HAL_OK)
        {
            return W25Qx_ERROR;
        }
        W25Qx_Disable();
        /* Wait the end of Flash writing */
        while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
            ;
        {
            /* Check for the Timeout */
            if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
            {
                return W25Qx_TIMEOUT;
            }
        }

        /* Update the address and size variables for next page programming */
        current_addr += current_size;
        pData += current_size;
        current_size = ((current_addr + W25Q128FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q128FV_PAGE_SIZE;
    } while (current_addr < end_addr);


    return W25Qx_OK;
}




void BSP_W25Qx_Erase_Blocks(uint32_t start,uint16_t num)
{
    uint16_t i=0;
    for(i=0; i<num; i++)
    {
        //BSP_W25Qx_Erase_Block((0x08000000 + start * 1024)+(1024 * i));

        MyFLASH_ErasePage((0x08000000 + start * 1024)+(1024 * i));
    }


}





/**********************************************************************************
 * ��������: ��������
 * �������: ��ַ
 */
uint8_t BSP_W25Qx_Erase_Block(uint32_t Address)
{
    uint8_t cmd[4];
    uint32_t tickstart = HAL_GetTick();
    cmd[0] = SECTOR_ERASE_CMD;
    cmd[1] = (uint8_t)(Address >> 16);
    cmd[2] = (uint8_t)(Address >> 8);
    cmd[3] = (uint8_t)(Address);

    /* Enable write operations */
    BSP_W25Qx_WriteEnable();

    /*Select the FLASH: Chip Select low */
    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);
    /*Deselect the FLASH: Chip Select high */
    W25Qx_Disable();

    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Q128FV_SECTOR_ERASE_MAX_TIME)
        {
            return W25Qx_TIMEOUT;
        }
    }
    return W25Qx_OK;
}


/**********************************************************************************
 * ��������: ��������
 * �������: ������������64��
 */
uint8_t BSP_W25Qx_Erase_Block64K(uint8_t block_number)
{
    uint8_t cmd[4];
    uint32_t tickstart = HAL_GetTick();
    cmd[0] = ERASE_BLOCK_CMD;
    cmd[1] = (uint8_t)(block_number * 64 * 1024 >> 16);
    cmd[2] = (uint8_t)(block_number * 64 * 1024 >> 8);
    cmd[3] = (uint8_t)(block_number * 64 * 1024);

    /* Enable write operations */
    BSP_W25Qx_WriteEnable();

    /*Select the FLASH: Chip Select low */
    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);
    /*Deselect the FLASH: Chip Select high */
    W25Qx_Disable();

    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Q128FV_SECTOR_ERASE_MAX_TIME)
        {
            return W25Qx_TIMEOUT;
        }
    }
    return W25Qx_OK;
}


/**********************************************************************************
 * ��������: оƬ����
 */
uint8_t BSP_W25Qx_Erase_Chip(void)
{
    uint8_t cmd[4];
    uint32_t tickstart = HAL_GetTick();
    cmd[0] = CHIP_ERASE_CMD;

    /* Enable write operations */
    BSP_W25Qx_WriteEnable();

    /*Select the FLASH: Chip Select low */
    W25Qx_Enable();
    /* Send the read ID command */
    HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);
    /*Deselect the FLASH: Chip Select high */
    W25Qx_Disable();

    /* Wait the end of Flash writing */
    while(BSP_W25Qx_GetStatus() != W25Qx_BUSY)
        ;
    {
        /* Check for the Timeout */
        if((HAL_GetTick() - tickstart) > W25Q128FV_BULK_ERASE_MAX_TIME)
        {
            return W25Qx_TIMEOUT;
        }
    }
    return W25Qx_OK;
}

