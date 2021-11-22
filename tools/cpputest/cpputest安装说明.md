# cpputest安装说明

# 1. 软件下载

[cpputest官网地址](https://cpputest.github.io/)

选择合适的版本进行安装.

windows平台下选择下载[the latest automatically released](https://github.com/cpputest/cpputest/releases/download/latest-passing-build/cpputest-latest.tar.gz)

ubuntu平台可选windows平台同样的版本，也可选择未进行编译过的版本，利用git进行下载：`git clone git://github.com/cpputest/cpputest.git`

## 2. 编译前准备

注：windows平台下载已进行编译过的版本，跳过此步骤。

安装编译依赖：`GNU autotools`

1. automake
2. autoconf
3. libtool

## 3. 编译

进入cpputest源码目录，按以下4步进行编译.

```
1. cd cpputest_build
2. autoreconf .. -i
3. ../configure
4. make
```

## 4. 添加环境变量

1. 将编译后的`lib`文件软链接至`/usr/local/lib`目录
2. 将cpputest的头文件软链接至`/usr/local/include`目录