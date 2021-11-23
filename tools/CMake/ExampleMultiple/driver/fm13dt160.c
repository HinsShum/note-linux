/**
 * @file driver\fm13dt160.c
 *
 * Copyright (C) 2021
 *
 * fm13dt160.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "fm13dt160.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include <string.h>

/*---------- macro ----------*/
#define _COMMAND_READ_MEMORY                            (0xB1)
#define _COMMAND_WRITE_MEMORY                           (0xB3)
#define _COMMAND_WRITE_REG                              (0xC5)
#define _COMMAND_READ_REG                               (0xC6)
#define _COMMAND_INIT_REGFILE                           (0xCE)
#define _COMMAND_OP_MODE_CHECK                          (0xCF)

/* VCC register defined
 */
#define _VCC_REG_ANA_CFG1                               (0xC012)
#define _VCC_REG_ANA_CFG2                               (0xC00B)
#define _VCC_REG_IO_INT_FLAG                            (0xC018)
#define _VCC_REG_INTERFACE_CTRL                         (0xC01C)
#define _VCC_REG_VDET_RESULT                            (0xC01E)

/* Battery register defined
 */
#define _BAT_REG_VDET_DELAY_START_CFG                   (0xC084)
#define _BAT_REG_VDET_STEP_CFG                          (0xC085)
#define _BAT_REG_VDET_TIMES_CNT                         (0xC091)
#define _BAT_REG_RTC_DELAY_CNT                          (0xC092)
#define _BAT_REG_RTC_FLOW_STATUS                        (0xC094)
#define _BAT_REG_RTC_FLOW_BLOCK_POINTER                 (0xC096)
#define _BAT_REG_SUMMAY_MAX_TEMPERATURE                 (0xC098)
#define _BAT_REG_SUMMAY_MIN_TEMPERATURE                 (0xC099)
#define _BAT_REG_SUMMAY_MAX_LIMIT_CNT                   (0xC09A)
#define _BAT_REG_SUMMAY_MIN_LIMIT_CNT                   (0xC09B)

/* Register bit defined
 */
#define _VCC_REG_IO_INT_FLAG_HF_IRQ                     (1 << 6)
#define _VCC_REG_IO_INT_FLAG_UHF_IRQ                    (1 << 5)
#define _VCC_REG_IO_INT_FLAG_RTC_IRQ                    (1 << 4)
#define _VCC_REG_IO_INT_FLAG_CLEAR_IRQ                  (1 << 0)
#define _VCC_REG_INTERFACE_CTRL_UHF                     (1 << 3)
#define _VCC_REG_INTERFACE_CTRL_IIC                     (1 << 2)
#define _VCC_REG_INTERFACE_CTRL_14443                   (1 << 1)
#define _VCC_REG_INTERFACE_CTRL_15693                   (1 << 0)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t fm13dt160_open(driver_t **pdrv);
static void fm13dt160_close(driver_t **pdrv);
static int32_t fm13dt160_write(driver_t **pdrv, void *buf, uint32_t address, uint32_t length);
static int32_t fm13dt160_read(driver_t **pdrv, void *buf, uint32_t address, uint32_t length);
static int32_t fm13dt160_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/* private functions
 */
static int32_t __fm13dt160_reg_write(fm13dt160_describe_t *pdesc, uint16_t reg, uint16_t val);
static int32_t __fm13dt160_reg_read(fm13dt160_describe_t *pdesc, uint16_t reg, uint16_t *pval);
static int32_t _ioctl_clear_interrupt(fm13dt160_describe_t *pdesc, void *args);
static int32_t _ioctl_initial_regfile(fm13dt160_describe_t *pdesc, void *args);

/*---------- type define ----------*/
typedef int32_t (*ioctl_cb_func_t)(fm13dt160_describe_t *pdesc, void *args);
typedef struct {
    uint32_t ioctl_cmd;
    ioctl_cb_func_t cb;
} ioctl_cb_t;

/*---------- variable ----------*/
DRIVER_DEFINED(fm13dt160, fm13dt160_open, fm13dt160_close, fm13dt160_write, fm13dt160_read, fm13dt160_ioctl, NULL);

static ioctl_cb_t ioctl_cb_array[] = {
    {IOCTL_FM13DT160_CLEAR_IRQ, _ioctl_clear_interrupt},
    {IOCTL_FM13DT160_INIT_REGFILE, _ioctl_initial_regfile}
};

/*---------- function ----------*/
static uint8_t __invert_8bits(uint8_t data)
{
    uint8_t retval = 0;

    for(uint8_t i = 0; i < 8; ++i) {
        if(data & (1 << i)) {
            retval |= (1 << (7 - i));
        }
    }

    return retval;
}

static uint16_t __invert_16bits(uint16_t data)
{
    uint16_t retval = 0;

    for(uint8_t i = 0; i < 16; ++i) {
        if(data & (1 << i)) {
            retval |= (1 << (15 - i));
        }
    }

    return retval;
}

static uint16_t __crc_14443_update(uint16_t crc, uint8_t byte)
{
    byte = (byte ^ (crc & 0xFF));
    byte = (byte ^ (byte << 4));
    crc = (crc >> 8) ^ ((uint16_t)byte << 8) ^ ((uint16_t)byte << 3) ^ ((uint16_t)byte >> 4);

    return crc;
}

static uint16_t _crc_14443a(uint8_t *pdata, uint16_t len)
{
    uint16_t crc = 0x6363;

    while(len--) {
        crc = __crc_14443_update(crc, __invert_8bits(*pdata++));
    }

    return __invert_16bits(crc);
}

static int32_t fm13dt160_open(driver_t **pdrv)
{
    fm13dt160_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    void *bus = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("FM13DT160 driver has no describe field\n");
            break;
        }
        retval = CY_EOK;
        if(pdesc->ops.init) {
            if(!pdesc->ops.init()) {
                __debug_error("FM13DT160 initialize failed\n");
                retval = CY_ERROR;
                break;
            }
        }
        /* bind to i2c bus */
        if(NULL == (bus = device_open(pdesc->bus_name))) {
            __debug_error("FM13DT160 bind i2c bus failed\n");
            if(pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            retval = CY_ERROR;
            break;
        }
        pdesc->bus = bus;
    } while(0);

    return retval;
}

static void fm13dt160_close(driver_t **pdrv)
{
    fm13dt160_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if(pdesc) {
        if(pdesc->bus) {
            device_close(pdesc->bus);
            pdesc->bus = NULL;
        }
        if(pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    }
}

/**
 * @brief Write memory to the fm13dt160.
 * @note The write operation time is preferably less than 1.6 seconds.
 * @param pdrv: the pointer of the device.
 * @param buf: the buffer will be writen.
 * @param address: the address of the memory.
 * @param length: the buffer length will be writen.
 * @retval If the args is wrong, the interface will return CY_E_WRONG_ARGS.
 *         If i2c bus has error occur, the interface will return CY_ERROR.
 *         If no error occur, the interface will return the actual length
 *         has be written.
 */
static int32_t fm13dt160_write(driver_t **pdrv, void *buf, uint32_t address, uint32_t length)
{
    fm13dt160_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    i2c_bus_msg_t msg = {0};
    uint8_t cmd[11] = {0};
    uint8_t *p = NULL;
    uint16_t crc = 0;
    uint32_t written_len = 0;
    uint8_t write_len = 0;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("FM13DT160 driver has no descirbe field\n");
            break;
        }
        if(!length) {
            __debug_error("FM13DT160 write length can not be 0\n");
            break;
        }
        if(pdesc->ops.pwctl) {
            pdesc->ops.pwctl(true);
            __delay_ms(10);
        }
        __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_IIC);
        retval = (int32_t)written_len;
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
        while(written_len < length) {
            p = cmd;
            *p++ = pdesc->address;
            *p++ = _COMMAND_WRITE_MEMORY;
            *p++ = (address >> 8) & 0xFF;
            *p++ = address & 0xFF;
            if(((address + (length - written_len)) & (~0x3)) != (address & (~0x03))) {
                write_len = ((address + 0x04) & (~0x03)) - address;
            } else {
                write_len = length - written_len;
            }
            *p++ = write_len - 1;
            memcpy(p, (char *)buf + written_len, write_len);
            p += write_len;
            crc = _crc_14443a(cmd, p - cmd);
            *p++ = (crc >> 8) & 0xFF;
            *p++ = crc & 0xFF;
            msg.type = I2C_BUS_TYPE_WRITE;
            msg.dev_addr = pdesc->address;
            msg.mem_addr = NULL;
            msg.mem_addr_counts = 0;
            msg.buf = cmd + 1;
            msg.len = p - cmd - 1;
            if(CY_EOK == device_write(pdesc->bus, &msg, 0, sizeof(msg))) {
                __delay_ms(15);
                msg.type = I2C_BUS_TYPE_SEQUENTIAL_READ;
                msg.dev_addr = pdesc->address;
                msg.mem_addr = NULL;
                msg.mem_addr_counts = 0;
                msg.buf = cmd;
                msg.len = 5;
                p = cmd;
                if(CY_EOK != device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
                    __debug_error("FM13DT160 write memory failed\n");
                    break;
                } else if(p[0] != 0x00 || p[1] != 0x00 || p[2] != 0x00) {
                    __debug_error("FM13DT160 write memory flag: %02X, result: %02X%02X\n", p[0], p[1], p[2]);
                    break;
                }
                written_len += write_len;
                address += write_len;
                retval = (int32_t)written_len;
            } else {
                __debug_error("FM13DT160 write memory command failed\n");
                break;
            }
        }
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
        __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_UHF | _VCC_REG_INTERFACE_CTRL_IIC);
        if(pdesc->ops.pwctl) {
            pdesc->ops.pwctl(false);
        }
    } while(0);

    return retval;
}

/**
 * @brief Read memory from the fm13dt160.
 * @note If want to read n bytes memory from the fm13dt160, the length must be n + 4,
 *       the actual buffer is `flag(1byte) + data(nbytes) + crc(2bytes)`
 * @param pdrv: the pointer of the device.
 * @param buf: the buffer to store the memory data.
 * @param address: the address of the memory.
 * @param length: the actual length + 4.
 * @retval If the args is wrong, the interface will return CY_E_WRONG_ARGS.
 *         If i2c bus has error occur, the interface will return CY_ERROR.
 *         If no error occur, the interface will return the actual length,
 *         for example, the actual length is 4, the interface will return 4.
 */
static int32_t fm13dt160_read(driver_t **pdrv, void *buf, uint32_t address, uint32_t length)
{
    fm13dt160_describe_t *pdesc = NULL;
    int32_t retval =CY_E_WRONG_ARGS;
    i2c_bus_msg_t msg = {0};
    uint8_t cmd[8] = {0};
    uint8_t *p = cmd;
    uint16_t crc = 0;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("FM13DT160 driver has no describe field\n");
            break;
        }
        if(!!(address & 0x03)) {
            __debug_error("FM13DT160 read address must be aligned with 4 bytes\n");
            break;
        }
        if(!!(length & 0x03)) {
            __debug_error("FM13DT160 read length must be aligned with 4 bytes\n");
            break;
        }
        if(length < 8) {
            __debug_error("FM13DT160 read the actual length can not be 0\n");
            break;
        }
        length -= 8;
        *p++ = pdesc->address;
        *p++ = _COMMAND_READ_MEMORY;
        *p++ = (address >> 8) & 0xFF;
        *p++ = address & 0xFF;
        *p++ = (length >> 8) & 0xFF;
        *p++ = length & 0xFF;
        crc = _crc_14443a(cmd, p - cmd);
        *p++ = (crc >> 8) & 0xFF;
        *p++ = crc & 0xFF;
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = NULL;
        msg.mem_addr_counts = 0;
        msg.buf = cmd + 1;
        msg.len = p - cmd - 1;
        if(pdesc->ops.pwctl) {
            pdesc->ops.pwctl(true);
            __delay_ms(10);
        }
        __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_IIC);
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
        if(CY_EOK == device_write(pdesc->bus, &msg, 0, sizeof(msg))) {
            __delay_us(100);
            length += 4;
            msg.type = I2C_BUS_TYPE_SEQUENTIAL_READ;
            msg.dev_addr = pdesc->address;
            msg.mem_addr = NULL;
            msg.mem_addr_counts = 0;
            msg.buf = buf;
            msg.len = length + 3;
            p = (uint8_t *)buf;
            if(CY_EOK != device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
                __debug_error("FM13DT160 read memory failed\n");
            } else if(p[0] == 0x00) {
                retval = (int32_t)length;
                for(uint32_t i = 0; i < length; ++i) {
                    p[i] = p[i + 1];
                }
            } else {
                retval = CY_ERROR;
                __debug_error("FM13DT160 read memory flag is error\n");
            }
        } else {
            __debug_error("FM13DT160 write read command failed\n");
        }
        device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
        __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_UHF | _VCC_REG_INTERFACE_CTRL_IIC);
        if(pdesc->ops.pwctl) {
            pdesc->ops.pwctl(false);
        }
    } while(0);

    return retval;
}

static int32_t __fm13dt160_reg_write(fm13dt160_describe_t *pdesc, uint16_t reg, uint16_t val)
{
    int32_t retval = CY_ERROR;
    i2c_bus_msg_t msg = {0};
    uint8_t cmd[8] = {0};
    uint8_t *p = cmd;
    uint16_t crc = 0;

    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
    do {
        *p++ = pdesc->address;
        *p++ = _COMMAND_WRITE_REG;
        *p++ = (reg >> 8) & 0xFF;
        *p++ = reg & 0xFF;
        *p++ = (val >> 8) & 0xFF;
        *p++ = val & 0xFF;
        crc = _crc_14443a(cmd, p - cmd);
        *p++ = (crc >> 8) & 0xFF;
        *p++ = crc & 0xFF;
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = NULL;
        msg.mem_addr_counts = 0;
        msg.buf = cmd + 1;
        msg.len = p - cmd - 1;
        if(CY_EOK != device_write(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 write reg command failed\n");
            break;
        }
        __delay_us(100);
        msg.type = I2C_BUS_TYPE_SEQUENTIAL_READ;
        msg.buf = cmd;
        msg.len = 5;
        if(CY_EOK != device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 get write reg command result failed\n");
            break;
        }
        if(cmd[0] != 0x00 || cmd[1] != 0x00 || cmd[2] != 0x00) {
            __debug_error("FM13DT160 write reg failed, err:%02X, result:%02X%02X\n", cmd[0], cmd[1], cmd[2]);
            break;
        }
        retval = CY_EOK;
        __debug_message("FM13DT160 write reg(%04X) value(%04X) ok\n", reg, val);
    } while(0);
    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);

    return retval;
}

static int32_t __fm13dt160_reg_read(fm13dt160_describe_t *pdesc, uint16_t reg, uint16_t *pval)
{
    int32_t retval = CY_ERROR;
    i2c_bus_msg_t msg = {0};
    uint8_t cmd[8] = {0};
    uint8_t *p = cmd;
    uint16_t crc = 0;

    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
    do {
        *p++ = pdesc->address;
        *p++ = _COMMAND_READ_REG;
        *p++ = (reg >> 8) & 0xFF;
        *p++ = reg & 0xFF;
        crc = _crc_14443a(cmd, p - cmd);
        *p++ = (crc >> 8) & 0xFF;
        *p++ = crc & 0xFF;
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = NULL;
        msg.mem_addr_counts = 0;
        msg.buf = cmd + 1;
        msg.len = p - cmd - 1;
        if(CY_EOK != device_write(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 write reg read command failed\n");
            break;
        }
        __delay_us(100);
        msg.type = I2C_BUS_TYPE_SEQUENTIAL_READ;
        msg.buf = cmd;
        msg.len = 5;
        if(CY_EOK != device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 get write reg read command result failed\n");
            break;
        }
        if(cmd[0] != 0x00) {
            __debug_error("FM13DT160 get reg value failed\n");
            break;
        }
        *pval = ((uint16_t)cmd[2] << 8) | cmd[1];
        __debug_message("FM13DT160 reg(%04X) value: %04X\n", reg, *pval);
        retval = CY_EOK;
    } while(0);
    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);

    return retval;
}

static int32_t _ioctl_initial_regfile(fm13dt160_describe_t *pdesc, void *args)
{
    int32_t retval = CY_ERROR;
    i2c_bus_msg_t msg = {0};
    uint8_t cmd[5] = {0};
    uint8_t *p = cmd;
    uint16_t crc = 0;

    if(pdesc->ops.pwctl) {
        pdesc->ops.pwctl(true);
        __delay_ms(10);
    }
    __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_IIC);
    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_LOCK, NULL);
    do {
        *p++ = pdesc->address;
        *p++ = _COMMAND_INIT_REGFILE;
        *p++ = 0x00;
        crc = _crc_14443a(cmd, p - cmd);
        *p++ = (crc >> 8) & 0xFF;
        *p++ = crc & 0xFF;
        msg.type = I2C_BUS_TYPE_WRITE;
        msg.dev_addr = pdesc->address;
        msg.mem_addr = NULL;
        msg.mem_addr_counts = 0;
        msg.buf = cmd + 1;
        msg.len = p - cmd - 1;
        if(CY_EOK != device_write(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 write regfile command failed\n");
            break;
        }
        __delay_us(600);
        msg.type = I2C_BUS_TYPE_SEQUENTIAL_READ;
        msg.buf = cmd;
        msg.len = 5;
        if(CY_EOK != device_read(pdesc->bus, &msg, 0, sizeof(msg))) {
            __debug_error("FM13DT160 get write regfile command result failed\n");
            break;
        }
        if(cmd[0] != 0x00 || cmd[1] != 0x00 || cmd[2] != 0x00) {
            __debug_error("FM13DY160 init regfile failed\n");
            break;
        }
        retval = CY_EOK;
    } while(0);
    device_ioctl(pdesc->bus, IOCTL_I2C_BUS_UNLOCK, NULL);
    __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_UHF | _VCC_REG_INTERFACE_CTRL_IIC);
    if(pdesc->ops.pwctl) {
        pdesc->ops.pwctl(false);
    }

    return retval;
}

static int32_t _ioctl_clear_interrupt(fm13dt160_describe_t *pdesc, void *args)
{
    int32_t retval = CY_EOK;

    if(pdesc->ops.pwctl) {
        pdesc->ops.pwctl(true);
        __delay_ms(10);
    }
    __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_IIC);
    retval = __fm13dt160_reg_write(pdesc, _VCC_REG_IO_INT_FLAG, 0);
    __fm13dt160_reg_write(pdesc, _VCC_REG_INTERFACE_CTRL, _VCC_REG_INTERFACE_CTRL_UHF | _VCC_REG_INTERFACE_CTRL_IIC);
    if(pdesc->ops.pwctl) {
        pdesc->ops.pwctl(false);
    }

    return retval;
}

static ioctl_cb_func_t _ioctl_cb_func_find(uint32_t ioctl_cmd)
{
    ioctl_cb_func_t cb = NULL;

    for(uint32_t i = 0; i < ARRAY_SIZE(ioctl_cb_array); ++i) {
        if(ioctl_cb_array[i].ioctl_cmd == ioctl_cmd) {
            cb = ioctl_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

static int32_t fm13dt160_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    fm13dt160_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;
    ioctl_cb_func_t cb = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if(!pdesc) {
            __debug_error("FM13DT160 has no describe field\n");
            break;
        }
        if(NULL == (cb = _ioctl_cb_func_find(cmd))) {
            __debug_error("FM13DT160 not support this command(%08X)\n", cmd);
            break;
        }
        retval = cb(pdesc, args);
    } while(0);

    return retval;
}
