## 格式化sd卡

1. 查看sd卡现有分区

```shell
hinsshum@hinsshum-virtual-machine:~/Desktop/stm32mp1/build$ lsblk 
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
loop0    7:0    0  54.7M  1 loop /snap/core18/1668
loop1    7:1    0   3.7M  1 loop /snap/gnome-system-monitor/135
loop2    7:2    0  91.4M  1 loop /snap/core/8689
loop3    7:3    0 160.2M  1 loop /snap/gnome-3-28-1804/116
loop4    7:4    0   956K  1 loop /snap/gnome-logs/81
loop5    7:5    0  54.6M  1 loop /snap/core18/1288
loop7    7:7    0  89.1M  1 loop /snap/core/8268
loop8    7:8    0 140.7M  1 loop /snap/gnome-3-26-1604/92
loop9    7:9    0  14.8M  1 loop /snap/gnome-characters/399
loop10   7:10   0 156.7M  1 loop /snap/gnome-3-28-1804/110
loop11   7:11   0  44.2M  1 loop /snap/gtk-common-themes/1353
loop12   7:12   0   3.7M  1 loop /snap/gnome-system-monitor/127
loop13   7:13   0  44.9M  1 loop /snap/gtk-common-themes/1440
loop15   7:15   0   4.2M  1 loop /snap/gnome-calculator/544
loop16   7:16   0 140.7M  1 loop /snap/gnome-3-26-1604/98
loop18   7:18   0  14.8M  1 loop /snap/gnome-characters/495
loop19   7:19   0   956K  1 loop /snap/gnome-logs/93
loop20   7:20   0   4.3M  1 loop /snap/gnome-calculator/704
sda      8:0    0    40G  0 disk 
└─sda1   8:1    0    40G  0 part /
sdb      8:16   1   1.9G  0 disk 
└─sdb1   8:17   1   1.9G  0 part /media/hinsshum/86B3-E671
sr0     11:0    1  1024M  0 rom
```

2. 卸载已挂载的sd卡分区

`sudo umount /dev/sdb1`

3. 清除先前的格式化

`sudo sgdisk -o /dev/sdb`

4. 重新格式化sd卡

```shell
sudo sgdisk --resize-table=128 -a 1 \
     -n 1:34:545      -c 1:fsbl1 \
     -n 2:546:1057    -c 2:fsbl2 \
     -n 3:1058:5153   -c 3:ssbl \
     -n 4:5154:136225 -c 4:bootfs \
     -n 5:136226:     -c 5:rootfs \
     -A 4:set:2 \
     -p /dev/sdb
```

5. 烧录镜像

```shell
sudo dd if=$1 of=/dev/sdb1 conv=fdatasync
sudo dd if=$1 of=/dev/sdb2 conv=fdatasync
sudo dd if=$2 of=/dev/sdb3 conv=fdatasync
```

