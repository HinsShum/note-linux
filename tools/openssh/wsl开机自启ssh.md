# WSL开机自启ssh服务

创建vbs文件

```
Set ws = CreateObject("Wscript.Shell")
ws.run "wsl -d Ubuntu-18.04 -u root /etc/init.d/ssh start", vbhide
```

打开windows运行，输入`shell:startup`

将vbs文件拖入文件夹