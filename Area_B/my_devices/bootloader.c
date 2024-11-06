#include "bootloader.h"


extern DMA_HandleTypeDef hdma_usart2_rx;
load_a load_A;


void write_otainfo(void)
{
    uint16_t i;
    uint16_t *wptr;
    wptr = (uint16_t *)&OTA_Info_t;
    MyFLASH_ErasePage(OTA_INFO_ADDR);
    MyFLASH_ErasePage(STM32_VERSION_ADDR);
    for(i=0; i<500; i++)
    {
        MyFLASH_ProgramHalfWord(OTA_INFO_ADDR + i * 2,*wptr);
        wptr++;
        HAL_Delay(5);
    }

}

void read_otainfo(void)
{
    uint16_t i;
    uint16_t *wptr;
    wptr = (uint16_t *)&OTA_Info_t;
    for(i=0; i<500; i++)
    {
        wptr[i]=MyFLASH_ReadHalfWord(OTA_INFO_ADDR + i * 2);

        //HAL_Delay(5);
    }

}



void bootloader_judge(void)
{
    read_otainfo();//�ϵ��ȡһ�ε�ǰ�汾����Ϣ
    if(0 == bootloader_enter(200))
    {
        if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)
        {
            wifi_printf("OTA����\r\n");
            boot_startflag |= UPDATA_A_FLAG;
            UpData_Info_t.W25Q32_BlockNumber = 0;
        }
        else
        {
            wifi_printf("��תϵͳ������A����\r\n");
            lOAD_A(STM32_A_START_ADDR);
        }

    }
    wifi_printf("����bootloader������\r\n");
    bootloader_info();


}

uint8_t bootloader_enter(uint8_t timeout)
{
    wifi_printf("%d��������Сд w ����bootloader\r\n",timeout/100);
    while(timeout--)
    {
        HAL_Delay(10);
    }
    if(WIFI_RX_Data_t.wifi_rxbuff[0] == 'w')
    {
        return 1;
    }
    return 0;
}

void bootloader_info(void)
{
    wifi_printf("\r\n");
    wifi_printf("��1������A��\r\n");
    wifi_printf("��2������IAP����A������\r\n");
    wifi_printf("��3������OTA�汾��\r\n");
    wifi_printf("��4����ѯOTA�汾��\r\n");
    wifi_printf("��5�����ⲿFLASH���س���\r\n");
    wifi_printf("��6��ʹ���ⲿFLASH�ڵĳ���\r\n");
    wifi_printf("��7������\r\n");
}

void bootloader_event(uint8_t *data,uint16_t datalen)
{
    int temp;
    if(0 == boot_startflag)
    {
        if((1 == datalen) && ('1' == data[0]))
        {
            wifi_printf("����A��\r\n");
            BSP_W25Qx_Erase_Blocks(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);
        }
        else if((1 == datalen) && ('2' == data[0]))
        {
            wifi_printf("ͨ��XmodemЭ�飬����IAP���س�����ʹ�� .bin��ʽ�ļ�\r\n");
            BSP_W25Qx_Erase_Blocks(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);
            boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG);
            UpData_Info_t.Xmodemtime = 0;
            UpData_Info_t.Xmodemnumber = 0;
        }
        else if((1 == datalen) && ('3' == data[0]))
        {
            wifi_printf("���ð汾��\r\n");
            boot_startflag |= SET_VERSION_FLAG;
        }
        else if((1 == datalen) && ('4' == data[0]))
        {
            wifi_printf("��ѯ�汾��\r\n");
            read_otainfo();
            //readota_version((uint16_t *)OTA_Info_t.ota_version);
            wifi_printf("��ѯ��ɣ���ǰ�汾��Ϊ��%s\r\n",OTA_Info_t.ota_version);
            bootloader_info();
        }
        else if((1 == datalen) && ('5' == data[0]))
        {
            wifi_printf("���ⲿflash���س�������Ҫʹ�õı��ţ�1~9��\r\n");
            boot_startflag |= CMD_5_FLAG;
        }
        else if((1 == datalen) && ('6' == data[0]))
        {
            wifi_printf("ʹ���ⲿflash�ڵĳ���������Ҫʹ�õı��\r\n");
            boot_startflag |= CMD_6_FLAG;
        }
        else if((1 == datalen) && ('7' == data[0]))
        {
            wifi_printf("����\r\n");
            HAL_Delay(100);
            NVIC_SystemReset();
        }

    }
    else if(boot_startflag&IAP_XMODEMDATA_FLAG)  //����XMODEMЭ��Ӵ�����������
    {
        if((133 == datalen) && (0x01 == data[0]))
        {
            boot_startflag &=~ IAP_XMODEM_FLAG;
            UpData_Info_t.Xmodemnumcrc = bootloader_crc16(&data[3],128);
            if(UpData_Info_t.Xmodemnumcrc == data[131]*256+data[132])
            {
                UpData_Info_t.Xmodemnumber++;
                memcpy(&UpData_Info_t.Updatabuff[((UpData_Info_t.Xmodemnumber-1)%(STM32_PAGE_SIZES/128))*128],&data[3],128);
                if(0 == (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))//���ݽ�����1k֮��
                {
                    if(boot_startflag & CMD_5_XMODEM_FLAG)//�ж��ǲ�������5
                    {
                        for(uint8_t i=0; i<4; i++)//д���ⲿflash
                        {
                            BSP_W25Qx_Write((uint8_t *)UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4,256);
                            //BSP_W25Qx_Page_Write(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);
                        }
                    }
                    else//д�뵥Ƭ��A��
                    {
                        BSP_W25Qx_Write_Blocks(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))-1) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,STM32_PAGE_SIZES);

                    }
                }
                wifi_printf("\x06");
            }
            else
            {
                wifi_printf("\x15");
            }
        }
        if((1 == datalen) && (0x04 == data[0]))
        {
            wifi_printf("\x06");
            if(0 != (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))
            {
                if(boot_startflag & CMD_5_XMODEM_FLAG)
                {
                    for(uint8_t i=0; i<4; i++)
                    {
                        BSP_W25Qx_Write((uint8_t *)UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4,256);//�ɵ� ����д�뺯�������� ����˵�����ݰ����Ȳ���  2024 11-6 21-26  (��֤OK)
                        //BSP_W25Qx_Page_Write(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);

                    }
                }
                else
                {
                    BSP_W25Qx_Write_Blocks(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,(UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128))*128);
                }
            }
            boot_startflag &=~ IAP_XMODEMDATA_FLAG;
            if(boot_startflag & CMD_5_XMODEM_FLAG)
            {
                boot_startflag &=~ CMD_5_XMODEM_FLAG;
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = UpData_Info_t.Xmodemnumber*128;
                write_otainfo();
                HAL_Delay(100);
                bootloader_info();
            }
            else
            {
                HAL_Delay(100);
                NVIC_SystemReset();
            }

        }

    }
    else if(boot_startflag & SET_VERSION_FLAG)
    {
        if(26 == datalen)
        {
            if((sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)==8))
            {
                memset(OTA_Info_t.ota_version,0,32);
                memcpy(OTA_Info_t.ota_version,data,26);
                write_otainfo();
                //writeota_version((uint16_t *)OTA_Info_t.ota_version);
                wifi_printf("������ɣ���ǰ�汾��Ϊ��%s\r\n",OTA_Info_t.ota_version);
                boot_startflag &=~ SET_VERSION_FLAG;
                bootloader_info();
            }
            else
                wifi_printf("���ð汾�Ÿ�ʽ����\r\n");
        }
        else
        {
            wifi_printf("���ð汾�ų��ȴ���\r\n");
        }

    }
    else if(boot_startflag & CMD_5_FLAG)
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0] - 0x30;
                boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG | CMD_5_XMODEM_FLAG);
                UpData_Info_t.Xmodemtime = 0;
                UpData_Info_t.Xmodemnumber = 0;
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = 0;
                //BSP_W25Qx_Erase_Block64K(UpData_Info_t.W25Q32_BlockNumber);//�ɵ� ���ܺ��������⣬���Ի��ɲ�������оƬ����  2024 11-6 21-26   (��֤OK)
                BSP_W25Qx_Erase_Chip();
                wifi_printf("ͨ��XmodemЭ�飬���ⲿflash��%d�������س�����ʹ�� .bin��ʽ�ļ�\r\n",UpData_Info_t.W25Q32_BlockNumber);
                boot_startflag &=~ CMD_5_FLAG;
            }
            else
            {
                wifi_printf("�����Ŵ���\r\n");
            }
        }

        else
        {
            wifi_printf("���ݳ��ȴ���\r\n");
        }
    }

    else if(boot_startflag & CMD_6_FLAG)
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0]-0x30;
                boot_startflag |= UPDATA_A_FLAG;
                boot_startflag &=~ CMD_6_FLAG;
            } else
            {
                wifi_printf("�����Ŵ���\r\n");
            }
        }
        else
        {
            wifi_printf("���ݳ��ȴ���\r\n");
        }
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
        bootloader_clear();
        MSR_SP(*(uint32_t *)addr);
        load_A = (load_a)*(uint32_t *)(addr+4);
        load_A();
    }
    else
    {
        wifi_printf("��תA��ʧ��\r\n");
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

/* only for xmoden crc*/
uint16_t bootloader_crc16(uint8_t * data,uint16_t datalen)
{
    uint16_t crcinit = 0x0000;
    uint16_t crcpoly = 0x1021;
    while(datalen--)
    {
        crcinit = (*data << 8) ^ crcinit;
        for(uint8_t i=0; i<8; i++)
        {
            if(crcinit&0x8000)
                crcinit = (crcinit << 1)^ crcpoly;
            else
                crcinit = (crcinit << 1);
        }
        data++;
    }
    return crcinit;


}
