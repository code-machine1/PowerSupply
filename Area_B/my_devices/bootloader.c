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
    read_otainfo();//�ϵ��ȡһ�ε�ǰ�汾����Ϣ
    if(0 == bootloader_enter(200))//�ж��Ƿ����boot loader������
    {
        if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)//�����־λĿǰû�����ã��������õ����������ķ�ʽ����A������⡣
        {
            wifi_printf("OTA����\r\n");
            boot_startflag |= UPDATA_A_FLAG;//A��������־λ������ʵ��д��main����������  ����Ų��
            UpData_Info_t.W25Q32_BlockNumber = 0;
        }
        else
        {
            wifi_printf("��תϵͳ������A����\r\n");
            lOAD_A(STM32_A_START_ADDR);   //��ת��A��
        }

    }
    wifi_printf("����bootloader������\r\n");
    bootloader_info();

}

uint8_t bootloader_enter(uint8_t timeout)//������boot loader�ļ��
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

void bootloader_info(void) //��ʾ����������
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

void bootloader_event(uint8_t *data,uint16_t datalen)  //�Խ��յ���������д���
{
    uint8_t i;//forѭ������
    int temp; //��ʱ���� ����������
    /* ���if������Ҫ�Ƕ��յ�����������жϣ�Ȼ����λ���Ӧ�ı�־λ */
    if(0 == boot_startflag)//
    {
        if((1 == datalen) && ('1' == data[0])) //����1
        {
            wifi_printf("����A��\r\n");
            stm32_eraseflash(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM);  //����A��
        }
        else if((1 == datalen) && ('2' == data[0])) //����2 
        {
            wifi_printf("ͨ��XmodemЭ�飬����IAP���س�����ʹ�� .bin��ʽ�ļ�\r\n");
            stm32_eraseflash(STM32_A_START_PAGE_NUM,STM32_A_PAGE_NUM); //����A��
            boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG); //��λXmodem��־λ
            UpData_Info_t.Xmodemtime = 0;    //��մ��������Ҫ�õ�������
            UpData_Info_t.Xmodemnumber = 0;  //��մ��������Ҫ�õ�������
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
    /* ǰ����λ���־λ��������Ͷ���ִ�еĴ��� */
    else if(boot_startflag&IAP_XMODEMDATA_FLAG)  //����XMODEMЭ��Ӵ�����������
    {
        if((133 == datalen) && (0x01 == data[0]))//XmodemЭ��һ���Ƿ�133���ֽڽ��� ����Ҫȥ��Э��
        {
            boot_startflag &=~ IAP_XMODEM_FLAG;
            UpData_Info_t.Xmodemnumcrc = bootloader_crc16(&data[3],128);//��133����ǰ�����ͺ����������ò�������Э������Ķ���������У������128���Ϳ����� ����Ҫȥ��Э��
            if(UpData_Info_t.Xmodemnumcrc == data[131]*256+data[132])//���У��ɹ�
            {
                UpData_Info_t.Xmodemnumber++;
                memcpy(&UpData_Info_t.Updatabuff[((UpData_Info_t.Xmodemnumber-1)%(STM32_PAGE_SIZES/128))*128],&data[3],128);
                if(0 == (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))//���ݽ�����1k֮��
                {
                    if(boot_startflag & CMD_5_XMODEM_FLAG)//�ж��ǲ�������5������5������2������������յ����ڷ����ĳ��������ֱ��д��A�����Ǵ浽�ⲿflash�У�
                    {
                        for(i=0; i<4; i++)//д���ⲿflash  ��4*256=1024������1k������д��
                        {
                            BSP_W25Qx_Write_Page(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8-1)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);//д�����ݵ�ƫ�����͵�ַ��ƫ����
                        }
                    }
                    else//д�뵥Ƭ��A����Ҳ�ǰ���1K�Ĵ�Сд��
                    {
                        stm32_writeflash(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))-1) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,STM32_PAGE_SIZES);//д�����ݵ�ƫ�����͵�ַ��ƫ����
                    }
                }
                wifi_printf("\x06");//�ⲿ����Xmodem��Э��Ҫ��  ����Ҫȥ��Э��
            }
            else
            {
                wifi_printf("\x15");//�ⲿ����Xmodem��Э��Ҫ��  ����Ҫȥ��Э��
            }
        }
        if((1 == datalen) && (0x04 == data[0]))//��������β�����ݣ���Ϊ�����ܳ����ǹ̶���С�� �����Բ��ܰ���1k����д����
        {
            wifi_printf("\x06");//�ⲿ����Xmodem��Э��Ҫ��  ����Ҫȥ��Э��
            if(0 != (UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128)))
            {
                if(boot_startflag & CMD_5_XMODEM_FLAG)//�ж��ǲ�������5������5������2������������յ����ڷ����ĳ��������ֱ��д��A�����Ǵ浽�ⲿflash�У�
                {
                    for(i=0; i<4; i++)
                    {
                        BSP_W25Qx_Write_Page(&UpData_Info_t.Updatabuff[i*256],(UpData_Info_t.Xmodemnumber/8)*4+i+UpData_Info_t.W25Q32_BlockNumber*64*4);//����Ҫ��ҳ������ƫ���� ֱ�ӽ��ⲿflash��
                    }
                }
                else
                {
                    stm32_writeflash(STM32_A_START_ADDR + ((UpData_Info_t.Xmodemnumber/(STM32_PAGE_SIZES/128))) * STM32_PAGE_SIZES,(uint32_t *)UpData_Info_t.Updatabuff,(UpData_Info_t.Xmodemnumber %(STM32_PAGE_SIZES/128))*128);//����Ҫ��ҳ������ƫ���� ֱ�ӵ���Ƭ����
                }
            }
            boot_startflag &=~ IAP_XMODEMDATA_FLAG;//�����־λ
            
            if(boot_startflag & CMD_5_XMODEM_FLAG)//�ж�������5��������2������5����Ҫ������������Ҫ�������ݵ��ڲ�flash
            {
                boot_startflag &=~ CMD_5_XMODEM_FLAG;
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = UpData_Info_t.Xmodemnumber*128;//��¼��ǰ����Ĵ�С
                write_otainfo();  //���浽�ڲ���flash
                HAL_Delay(100);   //�ȴ�һ�±������ ���п���
                bootloader_info();//���½���������
            }
            else//����2������
            {
                HAL_Delay(100);      //��ʱһ�� ���п���
                NVIC_SystemReset();  //����
            }

        }

    }
    else if(boot_startflag & SET_VERSION_FLAG)  //���ð汾�������
    {
        if(26 == datalen)//�汾�ų�������
        {
            if((sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)==8))//�汾�Ÿ�ʽ����
            {
                memset(OTA_Info_t.ota_version,0,32);     //����հ汾�Ŵ�ŵ�
                memcpy(OTA_Info_t.ota_version,data,26);  //�Ѱ汾��д�뵽�ṹ��
                write_otainfo();//д�뵽��Ƭ���ڲ�flash��
                wifi_printf("������ɣ���ǰ�汾��Ϊ��%s\r\n",OTA_Info_t.ota_version);//��ӡ��Ϣ
                boot_startflag &=~ SET_VERSION_FLAG;//��ձ�־λ
                bootloader_info();//��ʾboot loader ������
            }
            else
                wifi_printf("���ð汾�Ÿ�ʽ����\r\n");
        }
        else
        {
            wifi_printf("���ð汾�ų��ȴ���\r\n");
        }

    }
    else if(boot_startflag & CMD_5_FLAG)//����5����
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)//�������Ŀ��ŷ�Χ ���ַ�תʮ�����ƣ�
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0] - 0x30;  //�������ã��ַ�תʮ�����ƣ�
                boot_startflag |= (IAP_XMODEM_FLAG | IAP_XMODEMDATA_FLAG | CMD_5_XMODEM_FLAG);//��λ��־λ���ʹ����������������CMD_5_XMODEM_FLAG�����������ж�
                UpData_Info_t.Xmodemtime = 0;//��մ��������Ҫ�õ�������
                UpData_Info_t.Xmodemnumber = 0;//��մ��������Ҫ�õ�������
                OTA_Info_t.firelen[UpData_Info_t.W25Q32_BlockNumber] = 0;//���ota����ĳ����С��Ϣ
                BSP_W25Qx_Erase_Block64K(UpData_Info_t.W25Q32_BlockNumber);//�����ⲿflas�ж�Ӧ�Ŀ�
                wifi_printf("ͨ��XmodemЭ�飬���ⲿflash��%d�������س�����ʹ�� .bin��ʽ�ļ�\r\n",UpData_Info_t.W25Q32_BlockNumber);//��ӡһЩ��Ϣ
                boot_startflag &=~ CMD_5_FLAG;//����5��־λ����
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

    else if(boot_startflag & CMD_6_FLAG)//����6����
    {
        if(1 == datalen)
        {
            if((data[0]>=0x31)&&(data[0])<=0x39)//�������Ŀ��ŷ�Χ ���ַ�תʮ�����ƣ�
            {
                UpData_Info_t.W25Q32_BlockNumber = data[0]-0x30;//�������ã��ַ�תʮ�����ƣ�
                boot_startflag |= UPDATA_A_FLAG;//A��������־λ������ʵ��д��main����������  ����Ų��
                boot_startflag &=~ CMD_6_FLAG;  //����6��־λ����
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
