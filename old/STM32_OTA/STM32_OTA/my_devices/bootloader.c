#include "bootloader.h"
#include "main.h"
#include "usart.h"
void bootloader_judge(void)
{
    if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)
        wifi_printf("OTA����\r\n");
    else
        wifi_printf("��תϵͳ������A����\r\n");
}
