/**
 * @file /driver/w1/master/w1-gpio-char.c - GPIO w1 bus master
 *       driver, it's be registerd as a character device
 *
 * Copyright (C) 2020
 *
 * w1-gpio-char.c is free software: you can redistribute it and/or modify
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
 */

/*---------- includes ----------*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#include "../w1.h"
#include "../w1-gpio-char-ioctl.h"

/*---------- macro ----------*/
#define W1_GPIO_CHAR_DEVICE_NAME            "w1-gpio-char"

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int w1_gpio_char_open(struct inode *inode, struct file *file);
static long w1_gpio_char_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*---------- type define ----------*/
struct w1_gpio_platform_char_data {
    unsigned int out_pin;
    unsigned int in_pin;
    unsigned int pin_counts:4;
    unsigned int is_open_drain:1;
    void (*enable_external_pullup)(int enable);
    unsigned int ext_pullup_enable_pin;
    unsigned int pullup_duration;
};

enum w1_read_type {
    READ_DATA,
    READ_ROM,
};

struct w1_gpio_char_device {
    dev_t devno;
    struct class *w1_class;
    struct device *device;
};

struct w1_gpio_char_device_head {
    struct cdev cdev;
    int count;
    dev_t base;
    struct w1_gpio_char_device *pdev;
};

/*---------- variable ----------*/
static struct file_operations w1_gpio_char_ops = {
    .owner = THIS_MODULE,
    .open = w1_gpio_char_open,
    .unlocked_ioctl = w1_gpio_char_ioctl,
};

#if defined(CONFIG_OF)
static const struct of_device_id w1_gpio_dt_ids[] = {
    { .compatible = W1_GPIO_CHAR_DEVICE_NAME },
    {}
};
MODULE_DEVICE_TABLE(of, w1_gpio_dt_ids);
#endif

/*---------- function ----------*/
static u8 w1_gpio_set_pullup(void *data, int delay)
{
    struct w1_gpio_platform_char_data *pdata = data;

    if(delay) {
        pdata->pullup_duration = delay;
    } else {
        if(pdata->pullup_duration) {
            gpio_direction_output(pdata->out_pin, 1);
            msleep(pdata->pullup_duration);
            gpio_direction_input(pdata->out_pin);
        }
        pdata->pullup_duration = 0;
    }

    return 0;
}

static void w1_gpio_write_bit_dir(void *data, u8 bit)
{
    struct w1_gpio_platform_char_data *pdata = data;

    if(bit) {
        gpio_direction_input(pdata->out_pin);
    } else {
        gpio_direction_output(pdata->out_pin, 0);
    }
}

static void w1_gpio_write_bit_val(void *data, u8 bit)
{
    struct w1_gpio_platform_char_data *pdata = data;

    gpio_set_value(pdata->out_pin, bit);
}

static u8 w1_gpio_read_bit(void *data)
{
    struct w1_gpio_platform_char_data *pdata = data;
    unsigned int pin = (pdata->pin_counts == 1) ? pdata->out_pin : pdata->in_pin;

    return gpio_get_value(pin) ? 1 : 0;
}

static int _w1_gpio_char_reset_select(struct w1_master *master, u8 cmd, const u8 *rom)
{
    u8 match[10] = {0};
    u8 len = 0;

    if(w1_reset_bus(master)) {
        dev_err(&master->dev, "reset w1-sensor failed\n");
        return -ENXIO;
    }
    if(W1_READ_ROM == cmd) {
        match[0] = cmd;
        len = 1;
    } else if(W1_CONVERT_TEMP == cmd) {
        match[0] = W1_SKIP_ROM;
        match[1] = cmd;
        len = 2;
    } else if(W1_READ_SCRATCHPAD == cmd) {
        if(NULL != rom) {
            match[0] = W1_MATCH_ROM;
            memcpy(&match[1], rom, 8);
            match[9] = cmd;
            len = 10;
        } else {
            match[0] = W1_SKIP_ROM;
            match[1] = cmd;
            len = 2;
        }
    }
    w1_write_block(master, match, len);

    return 0;
}

static int _w1_gpio_char_read(struct w1_master *master, enum w1_read_type type, u8 *data)
{
    int len = (type == READ_ROM) ? 8 : 9;
    u8 crc = 0, count = 0, format[64] = {0}, format_len = 0;

    if(len != (count = w1_read_block(master, data, len))) {
        dev_err(&master->dev,
                "read from w1-sensor failed, read count is (%d) instead of 8\n", count);
        return -EPERM;
    }
    len--;
    crc = w1_calc_crc8(data, len);
    if((crc & 0x07) != (data[len] & 0x07)) {
        dev_err(&master->dev, "read from w1-sensor failed, crc error: %02X\n", crc);
        len = -EPERM;
    }
    format_len = sprintf(format, "raw: ");
    for(count = 0; count <= len; ++count) {
        format_len += sprintf(format + format_len, "%02X ", data[count]);
    }
    sprintf(format + format_len, ", crc: %02X\n", crc);
    dev_info(&master->dev, format);

    return len > 0 ? len + 1 : len;
}

static int w1_gpio_char_open(struct inode *inode, struct file *file)
{
    struct w1_gpio_char_device_head *phead = container_of(inode->i_cdev,
                                                          struct w1_gpio_char_device_head,
                                                          cdev);
    int base_minor = MINOR(phead->base), minor = MINOR(inode->i_rdev);
    int major = MAJOR(inode->i_rdev);
    struct w1_master *master = NULL;

    file->private_data = phead->pdev[minor - base_minor].device;
    master = dev_get_drvdata((struct device *)file->private_data);

    dev_info(&master->dev, "open ok, major:%d, minor:%d\n", major, minor);

    return 0;
}

static long w1_gpio_char_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct w1_master *master = dev_get_drvdata((struct device *)file->private_data);
    struct w1_gpio_char_ioctl_arg ioctl_arg;
    u8 ioctl_cmd = _W1_GPIO_CHAR_IOCTL_INVALID;
    int err = 0;
    int arg_len = 0;

    if(_W1_GPIO_CHAR_IOCTL_MAGIC != _IOC_TYPE(cmd)) {
        dev_err(&master->dev, "ioctl command invalid\n");
        return -EINVAL;
    }
    arg_len = _IOC_SIZE(cmd);
    ioctl_cmd = _IOC_NR(cmd);
    if(ioctl_cmd != _W1_GPIO_CHAR_IOCTL_CONVERT &&
       arg_len != sizeof(ioctl_arg)) {
        dev_err(&master->dev,
                "arg length must more then 8, but actval length is (%d)\n", arg_len);
        return -EINVAL;
    }
    if(ioctl_cmd != _W1_GPIO_CHAR_IOCTL_CONVERT &&
       0 != (err = copy_from_user(&ioctl_arg, (void *)arg, arg_len))) {
        dev_err(&master->dev, "copy from user space failed: (%d)\n", err);
        return  -EPERM;
    }

    switch(ioctl_cmd) {
        case _W1_GPIO_CHAR_IOCTL_CONVERT:
            err = _w1_gpio_char_reset_select(master, W1_CONVERT_TEMP, NULL);
            break;
        case _W1_GPIO_CHAR_IOCTL_READ_ROM:
            err = _w1_gpio_char_reset_select(master, W1_READ_ROM, NULL);
            if(!err) {
                err = _w1_gpio_char_read(master, READ_ROM, ioctl_arg.rom);
            }
            break;
        case _W1_GPIO_CHAR_IOCTL_READ_DATA_WITH_ROM:
            err = _w1_gpio_char_reset_select(master, W1_READ_SCRATCHPAD, NULL);
            if(!err) {
                err = _w1_gpio_char_read(master, READ_DATA, ioctl_arg.rom);
            }
            break;
        case _W1_GPIO_CHAR_IOCTL_READ_DATA:
            err = _w1_gpio_char_reset_select(master, W1_READ_SCRATCHPAD, ioctl_arg.rom);
            if(!err) {
                err = _w1_gpio_char_read(master, READ_DATA, ioctl_arg.rom);
            }
            break;
    }

    if(0 < err) {
        if(ioctl_cmd != _W1_GPIO_CHAR_IOCTL_CONVERT &&
           0 != (arg_len = copy_to_user((void *)arg, &ioctl_arg, arg_len))) {
            dev_err(&master->dev, "copy to user space failed: (%d)\n", arg_len);
            err = -EPERM;
        }
    }

    return err;
}

static int w1_gpio_probe_dt(struct platform_device *pdev)
{
    struct w1_gpio_platform_char_data *pdata = dev_get_platdata(&pdev->dev);
    struct device_node *parent = pdev->dev.of_node, *child = NULL;
    int gpio = 0, child_node_count = 0;

    child_node_count = of_get_child_count(parent);
    if(!child_node_count) {
        dev_err(&pdev->dev, "No child node exits\n");
        return -ENODATA;
    }

    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata) * child_node_count, GFP_KERNEL);
    if(!pdata) {
        return -ENOMEM;
    }
    pdev->dev.platform_data = pdata;

    for_each_child_of_node(parent, child) {
        if(of_get_property(child, "linux,open-drain", NULL)) {
            pdata->is_open_drain = 1;
        }
        gpio = of_get_named_gpio(child, "gpios-out", 0);
        if(gpio < 0) {
            if(gpio != -EPROBE_DEFER) {
                dev_err(&pdev->dev,
                        "Failed to parse gpio property for data pin (%d)\n",
                        gpio);
            }
            return gpio;
        }
        pdata->out_pin = gpio;
        pdata->pin_counts++;

        gpio = of_get_named_gpio(child, "gpios-out", 1);
        if(gpio == -EPROBE_DEFER) {
            return gpio;
        }
        /* ignore other errors as the pullup gpio is optional */
        pdata->ext_pullup_enable_pin = gpio;

        gpio = of_get_named_gpio(child, "gpios-in", 0);
        if(gpio == -EPROBE_DEFER) {
            return gpio;
        }
        /* ignore other errors as the input gpio is optional */
        pdata->in_pin = gpio;
        if(gpio >= 0) {
            pdata->pin_counts++;
        }

        pdata++;
    }

    return 0;
}

static int w1_gpio_probe(struct platform_device *pdev)
{
    struct w1_bus_master *bus_master;
    struct w1_master *master;
    struct w1_gpio_platform_char_data *pdata;
    struct w1_gpio_char_device_head *pdevice_head;
    struct w1_gpio_char_device *pdevice;
    int err, count = 0, i;
    dev_t w1_devno;
    struct class *w1_gpio_char_class;

    if(of_have_populated_dt()) {
        err = w1_gpio_probe_dt(pdev);
        if(err < 0) {
            goto w1_error;
        }
    }

    pdata = dev_get_platdata(&pdev->dev);
    if(!pdata) {
        dev_err(&pdev->dev, "No configuration data\n");
        err = -ENXIO;
        goto w1_error;
    }

    count = of_get_child_count(pdev->dev.of_node);
    if(!count) {
        dev_err(&pdev->dev, "No device defines in device tree\n");
        err = -ENODATA;
        goto w1_error;
    }

    pdevice_head = devm_kzalloc(&pdev->dev,
                                sizeof(struct w1_gpio_char_device_head) + \
                                (sizeof(struct w1_gpio_char_device) * count),
                                GFP_KERNEL);
    if(!pdevice_head) {
        dev_err(&pdev->dev, "Out of memory\n");
        err = -ENOMEM;
        goto w1_error;
    }
    pdevice_head->count = count;
    pdevice_head->pdev = (struct w1_gpio_char_device *)((u8 *)pdevice_head + \
                                                        sizeof(struct w1_gpio_char_device_head));
    pdevice = pdevice_head->pdev;

    /* register character device */
    err = alloc_chrdev_region(&w1_devno, 0, count, W1_GPIO_CHAR_DEVICE_NAME);
    if(err) {
        dev_err(&pdev->dev, "alloc_chrdev_region failed\n");
        goto w1_error;
    }
    pdevice_head->base = w1_devno;
    dev_info(&pdev->dev, "w1-gpio-char major:%d, baseminor:%d, count:%d\n",
             MAJOR(w1_devno), MINOR(w1_devno), count);

    cdev_init(&pdevice_head->cdev, &w1_gpio_char_ops);
    pdevice_head->cdev.owner = THIS_MODULE;
    err = cdev_add(&pdevice_head->cdev, w1_devno, count);
    if(err) {
        dev_err(&pdev->dev, "cdev_add failed\n");
        goto w1_cdev_add_err;
    }

    w1_gpio_char_class = class_create(THIS_MODULE, W1_GPIO_CHAR_DEVICE_NAME);
    if(IS_ERR(w1_gpio_char_class)) {
        dev_err(&pdev->dev, "class_create failed\n");
        err = PTR_ERR(w1_gpio_char_class);
        goto w1_class_create_err;
    }

    for(i = 0; i < count; ++i) {
        pdevice->w1_class = w1_gpio_char_class;
        pdevice->devno = MKDEV(MAJOR(w1_devno), MINOR(w1_devno) + i);
        dev_info(&pdev->dev, "device create major:%d, minor:%d\n",
                 MAJOR(pdevice->devno), MINOR(pdevice->devno));

        master = devm_kzalloc(&pdev->dev,
                              sizeof(struct w1_master) + sizeof(struct w1_bus_master),
                              GFP_KERNEL);
        if(!master) {
            dev_err(&pdev->dev, "Out of memory\n");
            err = -ENOMEM;
            goto w1_device_create_err;
        }
        bus_master = (struct w1_bus_master *)(master + 1);

        err = devm_gpio_request(&pdev->dev, pdata->out_pin, "w1 out");
        if(err) {
            dev_err(&pdev->dev, "gpio_request (pin out) failed\n");
            goto w1_device_create_err;
        }

        if(gpio_is_valid(pdata->ext_pullup_enable_pin)) {
            err = devm_gpio_request_one(&pdev->dev,
                                        pdata->ext_pullup_enable_pin,
                                        GPIOF_INIT_LOW, "w1 pullup");
            if(err < 0) {
                dev_err(&pdev->dev, "gpio_request_one (ext_pullup_enable_pin) failed\n");
                goto w1_device_create_err;
            }
        }

        if(gpio_is_valid(pdata->in_pin)) {
            err = devm_gpio_request(&pdev->dev, pdata->in_pin, "w1 in");
            if(err) {
                dev_err(&pdev->dev, "gpio_request (pin in) failed\n");
                goto w1_device_create_err;
            }
        }

        bus_master->data = pdata;
        bus_master->read_bit = w1_gpio_read_bit;
        if(pdata->is_open_drain) {
            gpio_direction_output(pdata->out_pin, 1);
            bus_master->write_bit = w1_gpio_write_bit_val;
        } else {
            gpio_direction_input(pdata->out_pin);
            bus_master->write_bit = w1_gpio_write_bit_dir;
            bus_master->set_pullup = w1_gpio_set_pullup;
        }

        if(pdata->enable_external_pullup) {
            pdata->enable_external_pullup(1);
        }

        if(gpio_is_valid(pdata->ext_pullup_enable_pin)) {
            gpio_set_value(pdata->ext_pullup_enable_pin, 1);
        }

        master->bus_master = bus_master;
        memcpy(&master->dev, &pdev->dev, sizeof(pdev->dev));

        pdevice->device = device_create(pdevice->w1_class, NULL,
                                        pdevice->devno, master, "w1-gpio-char%d", i);
        if(IS_ERR(pdevice->device)) {
            dev_err(&pdev->dev, "device_create failed\n");
            err = PTR_ERR(pdevice->device);
            goto w1_device_create_err;
        }

        pdevice++;
        pdata++;
    }
    platform_set_drvdata(pdev, pdevice_head);

    return 0;

w1_device_create_err:
    class_destroy(w1_gpio_char_class);
w1_class_create_err:
w1_cdev_add_err:
    cdev_del(&pdevice_head->cdev);
    unregister_chrdev_region(w1_devno, count);
w1_error:
    return err;
}

static int w1_gpio_remove(struct platform_device *pdev)
{
    struct w1_gpio_platform_char_data *pdata = dev_get_platdata(&pdev->dev);
    struct w1_gpio_char_device_head *phead = platform_get_drvdata(pdev);
    u8 i;

    if(pdata->enable_external_pullup) {
        pdata->enable_external_pullup(0);
    }

    if(gpio_is_valid(pdata->ext_pullup_enable_pin)) {
        gpio_set_value(pdata->ext_pullup_enable_pin, 0);
    }

    cdev_del(&phead->cdev);
    unregister_chrdev_region(phead->pdev->devno, phead->count);
    for(i = 0; i < phead->count; ++i) {
        device_destroy(phead->pdev[i].w1_class, phead->pdev[i].devno);
    }
    class_destroy(phead->pdev->w1_class);

    return 0;
}

static struct platform_driver w1_gpio_char_driver = {
    .driver = {
        .name = "w1-gpio-char",
        .of_match_table = of_match_ptr(w1_gpio_dt_ids),
    },
    .probe = w1_gpio_probe,
    .remove = w1_gpio_remove,
};

module_platform_driver(w1_gpio_char_driver);

MODULE_DESCRIPTION("GPIO w1 bus master character driver");
MODULE_AUTHOR("HinsShum <hinsshum@qq.com>");
MODULE_LICENSE("GPL");

