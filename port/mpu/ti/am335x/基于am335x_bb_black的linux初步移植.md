#  linux的初步移植记录

## 1. kernel源码下载

借用国内代理下载linux源码，版本选择linux-3.19.1

`https://mirror.tuna.tsinghua.edu.cn/kernel/v3.x/linux-3.19.1.tar.xz`

解压：

`xz -d linux-3.19.1.tar.xz`

`tar -xvf linux-3.19.1.tar`



## 2. 交叉编译工具链

工具链版本：`gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf`



## 3. 编译内核

### 3.1 导出环境变量

```bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-	# 编译工具路径已导出在PATH中
```

### 3.2 编译及配置

#### 3.2.1 配置 

查看现有配置文件：`ls arch/arm/configs`

![configs](images\configs.png)

查看发现没有与`am335x_beaglebone_black`开发板相关的配置文件，因此从TI的sdk中拷贝出与`beaglebone_black`相关的配置文件。

SDK地址：`https://github.com/beagleboard/kernel/tree/3.8`

下载该项目中的`configs/beaglebone`，重命名为`am335x_bbb_defconfig`放置在`arch/arm/configs`目录下

执行配置：

`make am335x_bbb_defconfig`

`make menuconfig`

#### 3.2.2 编译内核

`make LOADADDR=0x82000000 uImage -j4`

![uImage](images\uImage.png)

#### 3.2.3 编译设备树

`make am335x-boneblack.dtb`

#### 3.2.4 运行

拷贝image和dtb至tftp目录

`sudo cp arch/arm/boot/uImage /var/lib/tftpboot`

`sudo cp arch/arm/boot/dts/am335x-boneblack.dtb /var/lib/tftpboot`



tftp下载uImage和dtb：

![](images\tftp.png)



运行kernel：`bootm 82000000 - 83000000`

启动kernel成功，如下：

![bootm](images\bootm.png)



因未制作根文件系统，因此Kernel启动后在尝试挂载根文件系统时会失败，下一节说明如何制作根文件系统并挂载

![rootfs_failed](F:\Work_Directory\6.arch\arm\ti\am335x\reference\notes\images\rootfs_failed.png)



## 4. 根文件系统

说明：使用busybox制作最简单的根文件系统，开发板通过nfs挂载根文件系统（nfs相关设置自行百度查看，共享目录设置为$HOME/nfs/rootfs），需要注意的是，如使用ubunt18.04及以上版本的主机，在进行nfs相关设置的时候需要开启支持nfs2，因为uboot默认使用nfs2版本进行挂载。

### 4.1 下载busybox

下载busybox：`wget http://www.busybox.net/downloads/busybox-1.25.1.tar.bz2`

解压：`tar -jxvf busybox-1.25.1.tar.bz2`

### 4.2 配置及编译

```shell
export CROSS_COMPILE=arm-linux-gnueabihf-
cd busybox-1.25.1
make defconfig
make menuconfig		# 在menuconfig中选中 `Build BusyBox as a static binary (no shared libs)`
make install
```

### 4.3 构建根文件系统

```shell
mkdir $HOME/nfs/rootfs		# 创建根文件系统的目录
cp busybox1.25.1/_install/* $HOME/nfs/rootfs/ -r	# 拷贝busybox命令
cd $HOME/nfs/rootfs			# 进入该目录
mkdir dev mnt proc var tmp sys root lib opt	# 创建常用目录
sudo cp /opt/arm/gcc-linaro/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/lib/* lib/ -r
sudo mknod dev/tty1 c 4 1		# 创建tty设备
sudo mknod dev/tty2 c 4 2
sudo mknod dev/tty3 c 4 3
sudo mknod dev/tty4 c 4 4
cp busybox1.25.1/examples/bootfloppy/etc/* etc/ -r	# 拷贝最简单的init相关的文件到etc目录
```



## 5. 修改uboot的启动参数

开发主机ip：172.15.10.151

开发板ip：172.15.10.201

网关：172.15.10.1

子网掩码：255.255.255.0

kernel镜像：uImage

fdt镜像：am335x-boneblack.dtb

kernel加载地址：0x82000000

fdt加载地址：0x83000000

nfs目录：/home/cenyue/nfs/rootfs



修改uboot启动相关的环境变量

```shell
U-boot# set ipaddr 172.15.10.201
U-boot# set serverip 172.15.10.151
U-boot# set gatewayip 172.15.10.1
U-boot# set netmask 255.255.255.0
U-boot# set loadaddr 0x82000000
U-boot# set bootfile uImage
U-boot# set fdtaddr 0x83000000
U-boot# set fdtfile am335x-boneblack.dtb
U-boot# set rootpath /home/cenyue/nfs/rootfs
U-boot# set netargs 'setenv bootargs console=${console} ${optargs} root=/dev/nfs nfsroot=${serverip}:${rootpath},${nfsopts} rw ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}::eth0:off init=/linuxrc rootfstype=nfs rootwait=1'
U-boot# set netboot 'echo Booting from network ...; tftp ${loadaddr} ${bootfile}; tftp ${fdtaddr} ${fdtfile}; run netargs; bootm ${loadaddr} - ${fdtaddr}'
U-boot# set bootcmd 'run netboot'
U-boot# saveenv
U-boot# boot
```

登录成功：

![login](F:\Work_Directory\6.arch\arm\ti\am335x\reference\notes\images\login.png)

