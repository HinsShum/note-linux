# uboot2015.10 移植过程

Arch: arm

Soc: am3352

Board:  ant(与Beaglebone Black相似，emmc替换为Nand)



## 目录结构说明

```c
├── api			// 机器架构相关的用于应用编程的独立API
├── arch		// 架构相关的文件
├── board		// 板级相关的文件
├── common		// 独立于架构的misc类文件
├── configs		// 板级配置文件放置目录
├── disk		// 磁盘相关的驱动器分区处理代码
├── doc			// 文档目录
├── drivers		// Uboot驱动文件目录
├── dts			// dts目录，用于编译Uboot fdt
├── examples	// 独立应用程序的示例代码
├── fs			// 文件系统相关代码
├── include		// 头文件存放目录
├── lib			// 适用于所有架构
├── Licenses	// License
├── net			// 网络相关的文件
├── post		// Power on self test
├── scripts		// 脚本目录
├── spl			// spl 相关，编译后生成
├── test		// test相关
└── tools		// 用于编译生产Uboot的工具
```

从目录结构分析，与移植相关的目录有：arch、board、configs、include



## 移植

### board

board目录下的结构为`board/vendor/boardname`

vendor: 供应商名（sluan）

boardname: 开发板名（ant）

创建目录：`mkdir -p board/sluan/ant`

拷贝板级文件：`cp board/ti/am335x/* board/sluan/ant/`

修改u-boot.lds的text域中的built-in.o的路径为：`board/sluan/ant/built-in.o (.text*)`

修改Kconfig内容为：

```
if TARGET_AM335X_ANT

config SYS_BOARD
        default "ant"

config SYS_VENDOR
        default "sluan"

config SYS_SOC
        default "am33xx"

config SYS_CONFIG_NAME
        default "am335x_ant"

config CONS_INDEX
        int "UART used for console"
        range 1 6
        default 1
        help
          The AM335x SoC has a total of 6 UARTs (UART0 to UART5 as referenced
          in documentation, etc) available to it.  Depending on your specific
          board you may want something other than UART0 as for example the IDK
          uses UART3 so enter 4 here.

config NOR
        bool "Support for NOR flash"
        help
          The AM335x SoC supports having a NOR flash connected to the GPMC.
          In practice this is seen as a NOR flash module connected to the
          "memory cape" for the BeagleBone family.

config NOR_BOOT
        bool "Support for booting from NOR flash"
        depends on NOR
        help
          Enabling this will make a U-Boot binary that is capable of being
          booted via NOR.  In this case we will enable certain pinmux early
          as the ROM only partially sets up pinmux.  We also default to using
          NOR for environment.
endif
```



### configs

拷贝板级配置文件：`cp am335x_boneblack_defconfig am335x_ant_defconfig`

修改板级配置文件内容为：

```
CONFIG_SPL=y
CONFIG_SYS_EXTRA_OPTIONS="NAND"
CONFIG_CONS_INDEX=1
+S:CONFIG_ARM=y
+S:CONFIG_TARGET_AM335X_ANT=y
```



### arch

修改arch/arm/Kconfig，在内容中添加`ant`开发板的Kconfig支持和board路径：

```
config TARGET_AM335X_ANT
		bool "Support am335x_ant"
		select CPU_V7
		select SUPPORT_SPL
		
source "board/sluan/ant/Kconfig"
```



### include

添加板级头文件: `cp include/configs/am335x_evm.h include/configs/am335x_ant.h`

修改`am335x_ant.h`中关于链接脚本的路径为：

`#define CONFIG_SYS_LDSCRIPT             "board/sluan/ant/u-boot.lds"`



### 开发板相关代码修改

TI的Beaglebone Black开发板上有EEPROM，其存储了板级的信息用于用于根据板级信息选择不同的参数初始化相关硬件，我们的ANT开发板没有EEPROM，因此需要修改EEPROM相关代码。

修改board/sluan/ant/board.c中的**read_eeprom**函数为：

```C
static int read_eeprom(struct am335x_baseboard_id *header)
{
    header->magic = 0xEE3355AA;				// 设置魔数
    strcpy(header->name, "A335BNLT");		// 将板级信息中的名字设置为Beaglebone Black

	return 0;
}
```



ANT开发板使用Nand替换了Beaglebone Black的Emmc，因此需要配置Nand的相关引脚初始化

修改board/sluan/ant/mux.c中的**enable_board_pin_mux**函数为：

```C
void enable_board_pin_mux(struct am335x_baseboard_id *header)
{
	/* Do board-specific muxes. */
	if (board_is_bone(header)) {
		/* Beaglebone pinmux */
		configure_module_pin_mux(i2c1_pin_mux);
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
#if defined(CONFIG_NAND)
		configure_module_pin_mux(nand_pin_mux);
#elif defined(CONFIG_NOR)
		configure_module_pin_mux(bone_norcape_pin_mux);
#else
		configure_module_pin_mux(mmc1_pin_mux);
#endif
	} else if (board_is_gp_evm(header)) {
		/* General Purpose EVM */
		unsigned short profile = detect_daughter_board_profile();
		configure_module_pin_mux(rgmii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
		/* In profile #2 i2c1 and spi0 conflict. */
		if (profile & ~PROFILE_2)
			configure_module_pin_mux(i2c1_pin_mux);
		/* Profiles 2 & 3 don't have NAND */
#ifdef CONFIG_NAND
		if (profile & ~(PROFILE_2 | PROFILE_3))
			configure_module_pin_mux(nand_pin_mux);
#endif
		else if (profile == PROFILE_2) {
			configure_module_pin_mux(mmc1_pin_mux);
			configure_module_pin_mux(spi0_pin_mux);
		}
	} else if (board_is_idk(header)) {
		/* Industrial Motor Control (IDK) */
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_no_cd_pin_mux);
	} else if (board_is_evm_sk(header)) {
		/* Starter Kit EVM */
		configure_module_pin_mux(i2c1_pin_mux);
		configure_module_pin_mux(gpio0_7_pin_mux);
		configure_module_pin_mux(rgmii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux_sk_evm);
	} else if (board_is_bone_lt(header)) {
		/* Beaglebone LT pinmux */
		configure_module_pin_mux(i2c1_pin_mux);
		configure_module_pin_mux(mii1_pin_mux);
		configure_module_pin_mux(mmc0_pin_mux);
#if defined(CONFIG_NAND) && defined(CONFIG_EMMC_BOOT)
		configure_module_pin_mux(nand_pin_mux);
#elif defined(CONFIG_NOR) && defined(CONFIG_EMMC_BOOT)
        configure_module_pin_mux(bone_norcape_pin_mux);
#elif defined(CONFIG_NAND)								// 增加的代码
        configure_module_pin_mux(nand_pin_mux);			// 初始化Nand相关的引脚
#else
		configure_module_pin_mux(mmc1_pin_mux);
#endif
	} else {
		puts("Unknown board, cannot configure pinmux.");
		hang();
	}
}
```





## 编译、烧录及运行

在uboot根目录下编译：`make am335x_ant_defconfig && make -j8`

烧录至SD卡：

`sudo dd iflag=dsync oflag=dsync if=MLO of=/dev/sdb seek=0`

`sudo dd iflag=dysnc oflag-dsync if=u-boot.img of=/dev/sdb seek=768`



MLO的烧录地址可以为0x0\0x20000\0x40000\0x60000，seek大小为0x200，因此烧录MLO的seek可为0\256\512\768



u-boot.img的烧录地址在include/configs/ti_armv7_common.h中：

```C
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
```

因此烧录u-boot.img的seek为768



运行结果：

![运行结果](images\uboot_port.png)

