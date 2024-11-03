#include "bootloader.h"
#include "main.h"
#include "usart.h"
void bootloader_judge(void)
{
    if(OTA_UPDATA_STATUS == OTA_Info_t.ota_flag)
        wifi_printf("OTA更新\r\n");
    else
        wifi_printf("跳转系统分区（A区）\r\n");
}
