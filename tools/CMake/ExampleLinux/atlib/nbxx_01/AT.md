## 自动注网切换手动注网流程：

1.  AT+NCONFIG? 查询模块联网模式

2.  AT+NCONFIG=AUTOCONNECT,FALSE 关闭自动注网

3. AT+NRB 重启模块

4. AT+CFUN=1 启用全功能

5. AT+CGDCONT=1,"IP","ctnb" 设置APN

6. AT+CGATT=1 启动附着

7. AT+CGATT? 查询附着状态

8. AT+CSQ 查询信号值

9. AT+CEREG=2 设置查询注册信息模式

10. AT+CEREG? 查询注册信息

11. AT+COPS? 查询基站信息

12. AT+NCDP=112.93.129.154,5683 设置CDP服务器地址和端口

13. AT+NMGS=10, AA7232088D0320623399 发送消息

14. AT+NMGR 读取消息

