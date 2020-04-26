# Ubuntu18.04安装xrdp

一、安装xrdp

```
1. sudo apt install xrdp
2. http://www.c-nergy.be/downloads/install-xrdp-3.0.zip
3. unzip install-xrdp-3.0.zip
4. sudo chmod 777 Install-xrdp-3.0.sh
5. ./Install-xrdp-3.0.sh
6. shutdown -r now
```



二、添加规则文件至宿主目录

.xsession

```.xsession
env -u SESSION_MANAGER -u DBUS_SESSION_BUS_ADDRESS gnome-session
```

.xsessionrc

```.xsessionrc
export GNOME_SHELL_SESSION_MODE=ubuntu
export XDG_CURRENT_DESKTOP=ubuntu:GNOME
export XDG_CONFIG_DIRS=/etc/xdg/xdg-ubuntu:/etc/xdg
```





三、设置防火墙规则

`sudo ufw allow 3389`



四、使用windows的远程桌面连接ubutun，输入账号密码即可