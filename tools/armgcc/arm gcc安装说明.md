# ARM GCC安装说明

## 1. 下载安装包

[arm gcc下载地址](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

选择合适的版本进行下载，这里选择`10.3-2021.10`的linux版本。

## 2. 安装

### 2.1 解压缩

`sudo cp gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2`

`sudo tar -jxvf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 ` 

## 2.1 创建软链接

`sudo ln -s /usr/local/src/gcc-arm-none-eabi-10.3-2021.10/bin/* /usr/local/bin/`
