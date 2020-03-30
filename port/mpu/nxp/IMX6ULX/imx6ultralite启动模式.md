# 启动模式选择



IMX6ULX的启动模式由熔丝值和相关引脚的电平共同决定。



## 启动模式配置引脚

由`BOOT_MODE0`和`BOOT_MODE1`两个引脚的电平用于选择根据熔丝值还是相关引脚的信息来配置启动方式。

![boot_config](picture\boot_config.png)

熔丝值的地址主要位于0x450, 0x460, 0x470,和0x6D0。

其中地址0x450的熔丝值用于选择启动方式，如下图：

![boot_mode](picture\boot_mode.png)

若`BOOT_MODE[1:0]`选择`Internal Boot`，则`0x450`地址的值将被相关引脚的电平信息所覆盖：

![boot_pin](picture\boot_pin.png)

BOOT_MODE[1:0] = 0b00时，有两种启动流程分支：

1. BT_FUSE_SEL=0时，认为设备的启动设备（如Flash、SD/MMC等）未被编程，直接使用`Serial Downloader`模式进行启动
2. BT_FUSE_SEL=1时，根据熔丝值的设置值选择启动设备进行启动



## SD/MMC启动

imx6ulx具有两个 `USDHC  `通道，SD和MMC可以任意选择一个通道接入。

从SD/MMC启动时，`ROM CODE`会从启动设备拷贝4K字节的数据至内部的OCRAM，该4K字节中必须包含合法的`IVT`、`DCD`、`BOOT DATA STRUCTURES`的内容，保证设备的正常启动。

![IVT_offset](picture\IVTOffset.png)

在SD/MMC中，`IVT`从偏移1K处开始写入。

`Image Vector Table`组成：

![IVT](picture\IVT.png)