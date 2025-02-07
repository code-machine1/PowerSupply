实现程序更新功能，通过串口直接更新APP运行代码或将多个程序下载到外部flash中，自己选择想要的程序后再从外部flash中更新APP运行代码。
主要实现功能：

【1】擦除A区
【2】串口IAP下载A区程序
【3】设置OTA版本号
【4】查询OTA版本号
【5】向外部FLASH下载程序
【6】使用外部FLASH内的程序
【7】重启



## 1、程序分为A区（app）和B区（boot loader），例程基于STM32F103C8T6芯片编写。

​      B区地址从0x08000000到0x08009000
​      A区地址从0x08009000到开始0x08064E10

​     后续可根据实际使用情况在自行调节大小。保证大小够用，A区和B区地址不会冲突以及修改A区中断映射向量表。
![image-20241110165223289](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110165223289.png)

![image-20241110165405544](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110165405544.png)

![image-20241110165501989](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110165501989.png)

![image-20241110170505400](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110170505400.png)

## 2、连接串口2和上位机（SecureCRT 8.3）进行通信，波特率设置921600。单片机上电会从B区开始运行，2秒内没有更新操作（发送字符小写w）就会跳转到A区运行程序。

输入w后会进入bootloader命令行，输入对应的数字即可实现对应的功能。
![image-20241110170845861](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110170845861.png)



### （1）擦除A区代码：擦除后就无法跳转到A区运行了要注意，重新下载程序到A区即可。

![image-20241110172100422](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110172100422.png)

### （2）串口IAP下载A区程序：

在bootloader命令行界面输入数字2
![image-20241110171349437](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110171349437.png)

然后点击上位机的Transfer-->Send Xmodem选项，在弹出的选项框中选择A区的.bin文件。.bin文件的生成方法可以参考：[Keil生成并指定Bin和Hex文件的存放路径 - 附详细操作图文_keil生成bin文件-CSDN博客](https://blog.csdn.net/lnfiniteloop/article/details/111036611)
![image-20241110171513490](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110171513490.png)

![image-20241110171658041](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110171658041.png)

等待发送完成即可，发送完成后系统会自动重启，自此整个升级流程结束。
![image-20241110171747614](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110171747614.png)

## （3）设置/查询OTA版本号：

在bootloader命令行界面输入数字3，接着输入版本号即可。版本号格式：**VER-1.0.0-2024/11/5-11:00**  需严格按照此格式输入，否则无法设置成功。

![image-20241110173220304](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110173220304.png)

## （4）下载程序到外部flash中进行存储：

在bootloader命令行界面输入数字5，然后选择下载到flash中的第几个块中（一块的大小是64K，目前的设计有9块）。剩下选择.bin文件的操作与前面的串口下载的一样了。
![image-20241110173738914](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110173738914.png)

![image-20241110173823051](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110173823051.png)

## （5）使用外部FLASH内的程序：

在bootloader命令行界面输入数字6，然后输入flash的块编号即可。更新完成后会重新启动然后跳转到A区。
![image-20241110174008670](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110174008670.png)

![image-20241110174121246](https://github.com/code-machine1/PowerSupply/tree/master/imge/image-20241110174121246.png)



所有功能就演示完成了。















