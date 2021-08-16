# 如何在*atlib*的框架内添加新的模块

以添加`slm750`为例

## 1. 目录及文件创建

在目录`atlib`中创建子目录`slm750`并在其中创建四个文件分别是`slm750_if.c`, `slm750_if.h`, `_slm750_at.c`和`_slm750_at.h`



## 2. 函数编写

### slm750_if.c

根据具体业务需要实现针对于slm750模块的AT指令的函数如获取IMEI号，获取ICCID号等。

实现的函数均由`static`修饰且需符合`struct st_ryat_interface`结构体的规则（该结构体定义在`ryat.h`中）

定义该文件对外的接口变量`struct st_ryat_interface const ryat_interface_slm750`，并将实现的函数挂载至该变量下。

```c
struct st_ryat_interface const ryat_interface_slm750 = {
    .test = ryat_test,
    .close_echo = ryat_close_echo,
    .shutdown = ryat_shutdown,
    .get_csq = ryat_get_csq,
    .get_imei = ryat_get_imei,
    .get_iccid = ryat_get_iccid,
    .get_imsi = ryat_get_imsi,
    .attach = ryat_gprs_attach,
    .is_attached = ryat_gprs_is_attached,
    .set_apn = ryat_gprs_set_apn,
    .active = ryat_gprs_active,
    .deactivate = ryat_gprs_deactivate,
    .socket_init = ryat_socket_init,
    .socket_udp = ryat_socket_udp,
    .socket_shut = ryat_socket_shut,
    .write_data = ryat_write_data,
    .read_cache = ryat_read_cache,
    .get_creg_cell = ryat_get_creg_cell,
    .select_radio_access = ryat_gprs_select_radio_access,
    .get_radio_access = ryat_gprs_get_radio_access,
    .get_model = ryat_get_model,
};
```

最后将该变量`ryat_interface_slm750`添加至`ryat.c`文件下的`interface[]`数组中。

```c
static struct st_ryat_interface *interface[] = {
    [RYAT_MODULE_ID_L710] = &ryat_interface_l710,
    [RYAT_MODULE_ID_SLM750] = (struct st_ryat_interface *)&ryat_interface_slm750,
    [RYAT_MODULE_ID_G500] = (struct st_ryat_interface *)&ryat_interface_g500,
    [RYAT_MODULE_ID_U9X07] = (struct st_ryat_interface *)&ryat_interface_u9x07,
    [RYAT_MODULE_ID_BC26] = (struct st_ryat_interface *)&ryat_interface_bc26,
    [RYAT_MODULE_ID_NBXX_01] = (struct st_ryat_interface *)&ryat_interface_nbxx_01,
    [RYAT_MODULE_ID_SIM800] = (struct st_ryat_interface *)&ryat_interface_sim800,
};
```



## _slm750_at.c

定义slm750特有的AT命令返回值类型及返回值，该数组的最后必须以`{ RYAT_RET_NULL, NULL, 0}`结尾

```C
struct st_ryat_ret_describe const ryat_ret_tbl_slm750[] = {
    { RYAT_RET_OK, "OK", 2 },
    { RYAT_RET_ERROR, "ERROR", 5 },
    { RYAT_RET_CME, "+CME", 4 },
    { RYAT_RET_CSQ, "+CSQ", 4 },
    { RYAT_RET_GMM, "+GMM", 4 },
    { RYAT_RET_COPS, "+COPS", 5 },
    { RYAT_RET_CCID, "ICCID", 5 },
    { RYAT_RET_MIPCALL, "+MIPCALL", 8 },
    { RYAT_RET_MIPOPEN, "+MIPOPEN", 8 },
    { RYAT_RET_MIPCLOSE, "+MIPCLOSE", 9 },
    { RYAT_RET_CELL_INFO, " LAC_ID", 7 },
    { RYAT_RET_AT_P, "AT+", 3 },
    { RYAT_RET_P, "+", 1 },
    { RYAT_RET_NULL, NULL, 0}
};
```

将该变量添加至`ryat.c`文件下的`ryat_ret_tbl[]`数组中。

```c
static struct st_ryat_ret_describe *ryat_ret_tbl[] = {
    [RYAT_MODULE_ID_L710] = (struct st_ryat_ret_describe *)ryat_ret_tbl_l710,
    [RYAT_MODULE_ID_SLM750] = (struct st_ryat_ret_describe *)ryat_ret_tbl_slm750,
    [RYAT_MODULE_ID_G500] = (struct st_ryat_ret_describe *)ryat_ret_tbl_g500,
    [RYAT_MODULE_ID_U9X07] = (struct st_ryat_ret_describe *)ryat_ret_tbl_u9x07,
    [RYAT_MODULE_ID_BC26] = (struct st_ryat_ret_describe *)ryat_ret_tbl_bc26,
    [RYAT_MODULE_ID_NBXX_01] = (struct st_ryat_ret_describe *)ryat_ret_tbl_nbxx_01,
    [RYAT_MODULE_ID_SIM800] = (struct st_ryat_ret_describe *)ryat_ret_tbl_sim800,
};
```

---

实现slm750的数据接收函数`_slm750_at_get_data`，该函数由`slm750_if.c`中的`ryat_read_cache`调用

```c
static int32_t ryat_read_cache(uint8_t ch, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout)
{
    return _slm750_at_get_data(gp_at, ch, pdata, len, timeout);
}
```

```c
int32_t _slm750_at_get_data(struct st_ryat_describe *pat, uint8_t channel, uint8_t *pdata, uint16_t len, uint32_t timeout)
{
    int16_t tick = __ms_to_tick(timeout);
    char ch = 0;
    ryat_rx_line_t *pcurrent = &pat->cache.line[0];
    uint8_t state = STATE_GET_READ_PACKAGE_HEAD;
    char fmt[3] = {0};
    int16_t reallen = 0;

    ryat_reset_cache(pat);
    // +MIPRUDP=1,10.2.113.184,33259,494D4F4B
    // OK
    while(tick > 0 && state != STATE_END) {
        if(pat->port_read_char(&ch)) {
            switch(state) {
                case STATE_GET_READ_PACKAGE_HEAD: {
                    if(ch == '+' || pcurrent->len != 0) {
                        pcurrent->buf[pcurrent->len++] = ch;
                    }
                    if(ch == '=') {
                        if(!strncmp(pcurrent->buf, "+MIPRUDP", strlen("+MIPRUDP"))) {
                            state = STATE_GET_SOCKET_ID;
                            pcurrent++;
                        }
                    } else if(pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        pcurrent->len = 0;
                        pcurrent->buf[0] = 0x00;
                    }
                    break;
                }
                case STATE_GET_SOCKET_ID: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Socket id: %s\r\n", pcurrent->buf);
                        state = STATE_GET_LOCAL_IP;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get socket id failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_LOCAL_IP: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Local ip: %s\r\n", pcurrent->buf);
                        state = STATE_GET_LOCAL_PORT;
                        pcurrent++;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get local ip failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_LOCAL_PORT: {
                    pcurrent->buf[pcurrent->len++] = ch;
                    if(ch == ',') {
                        pcurrent->buf[pcurrent->len - 1] = '\0';
                        __ryat_debug("Local port: %s\r\n", pcurrent->buf);
                        state = STATE_GET_DATA;
                    } else if(ch == '\r' || ch == '\n' || ch == '\0' ||
                              pcurrent->len >= RYAT_COMMAND_LINE_LENGTH - 1) {
                        __ryat_debug("_slm750_at_get_data() get local port failed\r\n");
                        return RYAT_E_ERROR;
                    }
                    break;
                }
                case STATE_GET_DATA: {
                    if(isalnum(ch)) {
                        uint32_t tmp = 0;
                        uint8_t fmt_index = reallen % 2;

                        fmt[fmt_index] = ch;
                        if(fmt_index) {
                            sscanf(fmt, "%02X", &tmp);
                            pdata[reallen / 2] = (uint8_t)tmp;
                        }
                        reallen++;
                    } else {
                        state = STATE_END;
                    }
                    break;
                }
                default : break;
            }
        } else {
            tick--;
            pat->delay_tick(1);
        }
    }
    if(tick <= 0) {
        reallen = RYAT_E_ERROR;
    } else {
        reallen /= 2;
    }

    return reallen;
}
```

