# ESP32开发环境搭建

**本文档针对于手动搭建windows下的ESP32开发环境进行说明（Linux下方法类似）**

## 概述

ESP32开发环境需要安装一些必备工具才能围绕ESP32构建固件，包括：

* Python3
* Git
* 交叉编译工具链
* CMake
* Ninja

本文档主要通过***命令提示符***进行相关操作（若想通过图形化方式进行构建，请自行搜索相关资料）

## 安装Chocolatey

1. 管理员模式打开`PowerShell`
2. 输入`Get-ExecutionPolicy `，如果返回`Restricted `，则运行`Set-ExecutionPolicy Bypass -Scope Process`
3. 运行`Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))`
4. 输入`choco`查看是否安装成功

![1661223517245](img\安装Chocolatey.jpg)

## 其他工具安装

PowerShell下输入：

1. `choco install git python3 ninja cmake`
2. 设置cmake安装目录（C:\Program Files\CMake\bin）至环境变量

![](img\安装其他工具.png)

## ESP-IDF安装

### 下载SDK（ESP-IDF）

这里选择ESP-IDF的`v5.0-dev`这个分支作为构建的SDK版本，下载目录选择`C:\Users\xxx\Documents`

#### 克隆仓库

`git clone git@github.com:espressif/esp-idf.git`

![clone](img\clone.png)

#### 切换分支

`git checkout v5.0-dev`

#### 更新子模块

`git submodule update --init --rescursive`

![submodule](img\submodule.png)

#### 安装SDK相关的工具

替换下载源：`$env:IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"`

安装：`.\install.ps1`

![install](img\install.png)