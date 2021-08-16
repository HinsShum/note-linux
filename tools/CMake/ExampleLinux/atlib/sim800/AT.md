 1、AT+CPIN？查看SIM卡是否已准备好

2、AT+CGREG查看模块是否已经注册GPRS网络，如果没有需等待模块注册

3、AT+CSQ查询网络信号值，信号较好时，一般在29---31，若只有12、13，可能连接不上服务器

4、AT+CGATT?查询是否附着GPRS网络，若没有，可手动附着AT+CGATT=1

5、AT+CIPSHUT关闭移动场景。在初始化阶段不确定模块的IP STATE是什么状态，可统一做这个操作，它将关闭所有已连接的TCP/UDP连接，并且将IP STATE置为IP INITIATE，即初始化状态。因为模块需要在该状态才能执行激活移动场景的操作。若模块返回SHUT OK，则可继续一下步骤

6、AT+CIPMUX=1，若=1表示可使用多IP连接，若=0表示单链接

7、AT+CSTT="apn","username","password"  apn表示网络接入点，比如移动用CMNET，username和password表示入网的名户名和码，这将会在建立数据链路层的做验证的时候用到（PAP或者CHAP）

8、AT+CIICR激活移动场景。这条指令发出后，可能等待几十秒或更长，若模块返回OK，则表示移动场景已经激活。个人理解激活移动场景就是数据链路已经打通，可以在此基础上建立TCP/UDP连接了。

9、AT+CIFSR获取本地IP，必须有这一步才能使IP STATE变成IP STATUS，后续去建立具体的连接时才能成功。若指令正确，模块将返回随机IP

10、AT+CIPQSEND=1设置发送数据的回显格式，具体参考文档
