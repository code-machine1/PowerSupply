#include "bootloader.h"


extern DMA_HandleTypeDef hdma_usart2_rx;
load_a load_A;


void write_otainfo(void)
{
    uint16_t i;
    uint16_t *wptr;
    wptr = (uint16_t *)&OTA_Info_t;
    for(i=0; i<OTA_INFO_SIZE; i++)
    {
        MyFLASH_ProgramHalfWord(OTA_INFO_ADDR + i * 2,*wptr);
        wptr++;
    }

}

void read_otainfo(void)
{
    uint16_t i;
    uint16_t *wptr;
    wptr = (uint16_t *)&OTA_Info_t;
    for(i=0; i<OTA_INFO_SIZE; i++)
    {
        wptr[i]=MyFLASH_ReadHalfWord(OTA_INFO_ADDR + i * 2);
    }

}



void bootloader_judge(void)
{
    read_otainfo();//上电读取一次当前版本号信息
    if(0 == bootloader_enter(200))//判断是否进入boot loader命令行
    {
        if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)//这个标志位目前没有设置，后续会用到网络升级的方式中在A区做检测。
        {
            wifi_printf("OTA更新\r\n");
            boot_startflag |= UPDATA_A_FLAG;//A区升级标志位，具体实现写到main函数里面了  懒得挪了
            UpData_Info_t.W25Q32_BlockNumber = 0;
        }
        else
        {
            wifi_printf("跳转系统分区（A区）\r\n");
            lOAD_A(STM32_A_START_ADDR);   //跳转到A区
        }

    }
    wifi_printf("进入bootloader命令行\r\n");
    bootloader_info();

}

uint8_t bootloader_enter(uint8_t timeout)//做进入boot loader的检测
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

void bootloader_info(void) //显示操作命令行
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

void bootloader_event(uint8_t *data,uint16_t datalen)  //对接收到的命令进行处理
{
    uint8_t i;//for循环变量
    int temp; //临时变量 无特殊意义
    /* 这个if里面主要是对收到的命令进行判断，然后置位相对应的标志位 */
    if(0 == boot_startflag)//
    {
        if((1 == datalen) && ('1' == data[0])) //命令1
        {
            wifi_printf("擦除A区\r\n");
            stm32_eraseflash(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);  //擦除A区
        }
        else if((1 == datalen) && ('2' == data[0])) //命令2 
        {
            wifi_printf("通过Xmodem协议，串口IAP下载程序，请使用 .bin格式文件\r\n");
            stm32_eraseflash(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM); //擦除A区
            boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG); //置位Xmodem标志位
            UpData_Info_t.Xmodemtime = 0;    //清空待会儿下载要用到的数据
            UpData_Info_t.Xmodemnumber = 0;  //清空待会儿下载要用到的数据
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
    /* 前面置位完标志位后接下来就都是执行的代码 */
    else if(boot_startflag&IAP_XMODEMDATA_FLAG)  //启动XMODEM协议从串口下载数据
    {
        if((133 == datalen) && (0x01 == data[0]))//Xmodem协议一次是发133个字节进来 具体要去看协议
        {
            boot_startflag &=~ IAP_XMODEM_FLAG;
            UpData_Info_t.Xmodemnumcrc = bootloader_crc16(&data[3],128);//这133里面前三个和后三个我们用不到，是协议里面的东西，所以校验后面的128个就可以了 具体要去看协议
            if(UpData_Info_t.Xmodemnumcrc == data[131]*256+data[132])//如果校验成功
            {
                UpData_Info_t.Xmodemnumber++;
                memcpy(&UpData_Info_t.Updatabuff[((UpData_Info_t.Xmodemnumber-1)%(STM32_PAGE_SIZES/128))*128],&data[3],128);
                if(0 == (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))//数据接收完1k之后
                {
                    if(boot_startflag & CMD_5_XMODEM_FLAG)//判断是不是命令5（命令5和命令2的区别就在于收到串口发来的程序包后是直接写进A区还是存到外部flash中）
                    {
                        for(i=0; i<4; i++)//写入外部flash  （4*256=1024）按照1k的量来写入
                        {
                            BSP_W25Qx_Write_Page(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);//写入数据的偏移量和地址的偏移量
                        }
                    }
                    else//写入单片机A区，也是按照1K的大小写入
                    {
                        stm32_writeflash(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))-1) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,STM32_PAGE_SIZES);//写入数据的偏移量和地址的偏移量
                    }
                }
                wifi_printf("\x06");//这部分是Xmodem的协议要求  具体要去看协议
            }
            else
            {
                wifi_printf("\x15");//这部分是Xmodem的协议要求  具体要去看协议
            }
        }
        if((1 == datalen) && (0x04 == data[0]))//后面是收尾的数据，因为不可能程序都是固定大小的 ，所以不能按照1k的量写入了
        {
            wifi_printf("\x06");//这部分是Xmodem的协议要求  具体要去看协议
            if(0 != (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))
            {
                if(boot_startflag & CMD_5_XMODEM_FLAG)//判断是不是命令5（命令5和命令2的区别就在于收到串口发来的程序包后是直接写进A区还是存到外部flash中）
                {
                    for(i=0; i<4; i++)
                    {
                        BSP_W25Qx_Write_Page(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);//不需要对页数进行偏移了 直接进外部flash里
                    }
                }
                else
                {
                    stm32_writeflash(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,(UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128))*128);//不需要对页数进行偏移了 直接到单片机里
                }
            }
            boot_startflag &=~ IAP_XMODEMDATA_FLAG;//清理标志位
            
            if(boot_startflag & CMD_5_XMODEM_FLAG)//判断是命令5还是命令2，命令5不需要重启，但是需要保存数据到内部flash
            {
                boot_startflag &=~ CMD_5_XMODEM_FLAG;
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = UpData_Info_t.Xmodemnumber*128;//记录当前程序的大小
                write_otainfo();  //保存到内部的flash
                HAL_Delay(100);   //等待一下保存完成 可有可无
                bootloader_info();//重新进入命令行
            }
            else//命令2就重启
            {
                HAL_Delay(100);      //延时一下 可有可无
                NVIC_SystemReset();  //重启
            }

        }

    }
    else if(boot_startflag & SET_VERSION_FLAG)  //设置版本号命令处理
    {
        if(26 == datalen)//版本号长度限制
        {
            if((sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)==8))//版本号格式限制
            {
                memset(OTA_Info_t.ota_version,0,32);     //先清空版本号存放的
                memcpy(OTA_Info_t.ota_version,data,26);  //把版本号写入到结构体
                write_otainfo();//写入到单片机内部flash中
                wifi_printf("设置完成，当前版本号为：%s\r\n",OTA_Info_t.ota_version);//打印信息
                boot_startflag &=~ SET_VERSION_FLAG;//清空标志位
                bootloader_info();//显示boot loader 命令行
            }
            else
                wifi_printf("设置版本号格式错误\r\n");
        }
        else
        {
            wifi_printf("设置版本号长度错误\r\n");
        }

    }
    else if(boot_startflag & CMD_5_FLAG)//命令5处理
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)//检测输入的块编号范围 （字符转十六进制）
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0] - 0x30;  //块编号设置（字符转十六进制）
                boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG | CMD_5_XMODEM_FLAG);//置位标志位，和串口升级的区别多了CMD_5_XMODEM_FLAG，用来后面判断
                UpData_Info_t.Xmodemtime = 0;//清空待会儿下载要用到的数据
                UpData_Info_t.Xmodemnumber = 0;//清空待会儿下载要用到的数据
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = 0;//清空ota保存的程序大小信息
                BSP_W25Qx_Erase_Block64K(UpData_Info_t.W25Q32_BlockNumber);//擦除外部flas中对应的块
                wifi_printf("通过Xmodem协议，向外部flash第%d个块下载程序，请使用 .bin格式文件\r\n",UpData_Info_t.W25Q32_BlockNumber);//打印一些信息
                boot_startflag &=~ CMD_5_FLAG;//命令5标志位清零
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

    else if(boot_startflag & CMD_6_FLAG)//命令6处理
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)//检测输入的块编号范围 （字符转十六进制）
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0]-0x30;//块编号设置（字符转十六进制）
                boot_startflag |= UPDATA_A_FLAG;//A区升级标志位，具体实现写到main函数里面了  懒得挪了
                boot_startflag &=~ CMD_6_FLAG;  //命令6标志位清零
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
    HAL_SPI_DeInit (&hspi2);
    HAL_DMA_DeInit (&hdma_usart2_rx);
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
