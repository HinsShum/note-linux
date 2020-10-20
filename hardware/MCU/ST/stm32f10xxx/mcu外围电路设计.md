# MCU外围电路设计

## 1. 复位电路

​		根据stm32f1xx系列芯片硬件设计参考手册([文件编号：AN2586](https://www.st.com/resource/en/application_note/cd00164185-getting-started-with-stm32f10xxx-hardware-development-stmicroelectronics.pdf))介绍，stm32f1xx不需要额外的复位电路即可正常上电启动，只是建议在复位引脚下拉一个0.1uF的电容，来保护器件免受寄生复位的影响来提高EMS性能。

​		因`NREST`引脚内部有上拉电阻，因此通过内部电阻对外部下拉电容的充放电会增加器件的功耗，如若想要降低功耗，可将下拉电容更换至最小10nF。

![复位电路](image/复位电路.png)

​		根据手册内容，每个复位源对器件进行复位时，都会使脉冲发生器保持最少20uS的的复位脉冲，即保持了`NRST`的持续接地，如果在外部有接看门狗，则此时的看门狗复位引脚输出为高电平，为了避免电流灌入`NRST`，需要在看门狗的复位引脚与`NRST`之间接入限流电阻，因为器件的输入电流大小限制在±5mA，因此通常限流电阻大小为10K。

![输入电流限制](image/输入电流限制.png)

​		或者使用输出电流较小的看门狗芯片，如`TPL5010`，其IO输出电流大小典型值为35nA。

![TPL5010输出电流](image/TPL5010输出电流.png)



## 2. boot引脚

stm32f1xx系列通常有三种启动方式，根据boot[0-1]引脚的接法不同，分为：

1. NorFlash启动
2. SystemMemory启动
3. SRAM启动

![boot_configure](image/boot_configure.png)

其通常的接法如下：

![boot引脚接法](image/boot引脚接法.png)

## 3. 晶体振荡器

### 3.1 无源晶体振荡器电路设计要点

