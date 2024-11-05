#include "MyFLASH.h"                 // Device header
#include "stm32f1xx_hal_flash_ex.h"
/**
  * 函    数：FLASH读取一个32位的字
  * 参    数：Address 要读取数据的字地址
  * 返 回 值：指定地址下的数据
  */
uint32_t MyFLASH_ReadWord(uint32_t Address)
{
    return *((__IO uint32_t *)(Address));   //使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH读取一个16位的半字
  * 参    数：Address 要读取数据的半字地址
  * 返 回 值：指定地址下的数据
  */
uint16_t MyFLASH_ReadHalfWord(uint32_t Address)
{
    return *((__IO uint16_t *)(Address));   //使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH读取一个8位的字节
  * 参    数：Address 要读取数据的字节地址
  * 返 回 值：指定地址下的数据
  */
uint8_t MyFLASH_ReadByte(uint32_t Address)
{
    return *((__IO uint8_t *)(Address));    //使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH全擦除
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，FLASH的所有页都会被擦除，包括程序文件本身，擦除后，程序将不复存在
  */
//void MyFLASH_EraseAllPages(void)
//{
//  HAL_FLASH_Unlock();                 //解锁
//  FLASH_EraseAllPages();          //全擦除
//  HAL_FLASH_Lock();                   //加锁
//}

/**
  * 函    数：FLASH页擦除
  * 参    数：PageAddress 要擦除页的页地址
  * 返 回 值：无
  */
void MyFLASH_ErasePage(uint32_t PageAddress)
{
    HAL_FLASH_Unlock();                 //解锁
    uint32_t Flash_Add = PageAddress;
    FLASH_EraseInitTypeDef My_Flash;  //声明FLASH_EraseInitTypeDef 结构体为 My_Flash
    HAL_FLASH_Unlock();               //解锁Flash
    My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只做擦除操作
    My_Flash.PageAddress = Flash_Add;  //声明要擦除的地址
    My_Flash.NbPages = 1;                        //说明要擦除的页数，此参数必须是Min_Data = 1和Max_Data =(最大页数-初始页的值)之间的值
    uint32_t PageError = 0;                    //设置PageError,如果出现错误这个变量会被设置为出错的FLASH地址
    HAL_FLASHEx_Erase(&My_Flash, &PageError);  //调用擦除函数擦除
    HAL_FLASH_Lock();
}

/**
  * 函    数：FLASH编程字
  * 参    数：Address 要写入数据的字地址
  * 参    数：Data 要写入的32位数据
  * 返 回 值：无
  */
void MyFLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
    HAL_FLASH_Unlock();                         //解锁
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address,Data);     //编程字
    HAL_FLASH_Lock();                           //加锁
}

/**
  * 函    数：FLASH编程半字
  * 参    数：Address 要写入数据的半字地址
  * 参    数：Data 要写入的16位数据
  * 返 回 值：无
  */
void MyFLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
    HAL_FLASH_Unlock();                         //解锁
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,Data);//编程半字
    HAL_FLASH_Lock();                           //加锁
}
