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
    read_otainfo();//上电读取一次当前版本号信息
    if(0 == bootloader_enter(200))
    {
        if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)
        {
            wifi_printf("OTA更新\r\n");
            boot_startflag |= UPDATA_A_FLAG;
            UpData_Info_t.W25Q32_BlockNumber = 0;
        }
        else
        {
            wifi_printf("跳转系统分区（A区）\r\n");
            lOAD_A(STM32_A_START_ADDR);
        }

    }
    wifi_printf("进入bootloader命令行\r\n");
    bootloader_info();


}

uint8_t bootloader_enter(uint8_t timeout)
{
    wifi_printf("%d秒内输入小写 w 进入bootloader\r\n",timeout/100);
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
    wifi_printf("【1】擦除A区\r\n");
    wifi_printf("【2】串口IAP下载A区程序\r\n");
    wifi_printf("【3】设置OTA版本号\r\n");
    wifi_printf("【4】查询OTA版本号\r\n");
    wifi_printf("【5】向外部FLASH下载程序\r\n");
    wifi_printf("【6】使用外部FLASH内的程序\r\n");
    wifi_printf("【7】重启\r\n");
}

void bootloader_event(uint8_t *data,uint16_t datalen)
{
    int temp;
    if(0 == boot_startflag)
    {
        if((1 == datalen) && ('1' == data[0]))
        {
            wifi_printf("擦除A区\r\n");
            BSP_W25Qx_Erase_Blocks(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);
        }
        else if((1 == datalen) && ('2' == data[0]))
        {
            wifi_printf("通过Xmodem协议，串口IAP下载程序，请使用 .bin格式文件\r\n");
            BSP_W25Qx_Erase_Blocks(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);
            boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG);
            UpData_Info_t.Xmodemtime = 0;
            UpData_Info_t.Xmodemnumber = 0;
        }
        else if((1 == datalen) && ('3' == data[0]))
        {
            wifi_printf("设置版本号\r\n");
            boot_startflag |= SET_VERSION_FLAG;
        }
        else if((1 == datalen) && ('4' == data[0]))
        {
            wifi_printf("查询版本号\r\n");
            read_otainfo();
            //readota_version((uint16_t *)OTA_Info_t.ota_version);
            wifi_printf("查询完成，当前版本号为：%s\r\n",OTA_Info_t.ota_version);
            bootloader_info();
        }
        else if((1 == datalen) && ('5' == data[0]))
        {
            wifi_printf("向外部flash下载程序，输入要使用的编块号（1~9）\r\n");
            boot_startflag |= CMD_5_FLAG;
        }
        else if((1 == datalen) && ('6' == data[0]))
        {
            wifi_printf("使用外部flash内的程序，输入需要使用的编号\r\n");
            boot_startflag |= CMD_6_FLAG;
        }
        else if((1 == datalen) && ('7' == data[0]))
        {
            wifi_printf("重启\r\n");
            HAL_Delay(100);
            NVIC_SystemReset();
        }

    }
    else if(boot_startflag&IAP_XMODEMDATA_FLAG)  //启动XMODEM协议从串口下载数据
    {
        if((133 == datalen) && (0x01 == data[0]))
        {
            boot_startflag &=~ IAP_XMODEM_FLAG;
            UpData_Info_t.Xmodemnumcrc = bootloader_crc16(&data[3],128);
            if(UpData_Info_t.Xmodemnumcrc == data[131]*256+data[132])
            {
                UpData_Info_t.Xmodemnumber++;
                memcpy(&UpData_Info_t.Updatabuff[((UpData_Info_t.Xmodemnumber-1)%(STM32_PAGE_SIZES/128))*128],&data[3],128);
                if(0 == (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))//数据接收完1k之后
                {
                    if(boot_startflag & CMD_5_XMODEM_FLAG)//判断是不是命令5
                    {
                        for(uint8_t i=0; i<4; i++)//写入外部flash
                        {
                            BSP_W25Qx_Write((uint8_t *)UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4,256);
                            //BSP_W25Qx_Page_Write(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);
                        }
                    }
                    else//写入单片机A区
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
                        BSP_W25Qx_Write((uint8_t *)UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4,256);//疑点 可能写入函数有问题 报错说明数据包长度不对  2024 11-6 21-26  (验证OK)
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
                wifi_printf("设置完成，当前版本号为：%s\r\n",OTA_Info_t.ota_version);
                boot_startflag &=~ SET_VERSION_FLAG;
                bootloader_info();
            }
            else
                wifi_printf("设置版本号格式错误\r\n");
        }
        else
        {
            wifi_printf("设置版本号长度错误\r\n");
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
                //BSP_W25Qx_Erase_Block64K(UpData_Info_t.W25Q32_BlockNumber);//疑点 可能函数有问题，可以换成擦除整个芯片试试  2024 11-6 21-26   (验证OK)
                BSP_W25Qx_Erase_Chip();
                wifi_printf("通过Xmodem协议，向外部flash第%d个块下载程序，请使用 .bin格式文件\r\n",UpData_Info_t.W25Q32_BlockNumber);
                boot_startflag &=~ CMD_5_FLAG;
            }
            else
            {
                wifi_printf("输入编号错误\r\n");
            }
        }

        else
        {
            wifi_printf("数据长度错误\r\n");
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
                wifi_printf("输入编号错误\r\n");
            }
        }
        else
        {
            wifi_printf("数据长度错误\r\n");
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
        wifi_printf("跳转A区失败\r\n");
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
