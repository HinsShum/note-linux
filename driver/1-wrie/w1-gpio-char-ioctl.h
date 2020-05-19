/**
 * @file driver/w1/w1-gpio-char-ioctl.h
 *
 * Copyright (C) 2020
 *
 * w1-gpio-char-ioctl.h is free software: you can redistribute it and/or modify
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
#ifndef __DRIVER_W1_W1_GPIO_CHAR_IOCTL_H
#define __DRIVER_W1_W1_GPIO_CHAR_IOCTL_H

/*---------- includes ----------*/
#include <linux/ioctl.h>
#include <linux/types.h>

/*---------- macro ----------*/
#define _W1_GPIO_CHAR_IOCTL_MAGIC                   ('W')
#define _W1_GPIO_CHAR_IOCTL_CONVERT                 (0)
#define _W1_GPIO_CHAR_IOCTL_READ_ROM                (1)
#define _W1_GPIO_CHAR_IOCTL_READ_DATA_WITH_ROM      (2)
#define _W1_GPIO_CHAR_IOCTL_READ_DATA               (3)
#define _W1_GPIO_CHAR_IOCTL_INVALID                 (255)

#define W1_GPIO_CHAR_IOCTL_CONVERT                  _IO(_W1_GPIO_CHAR_IOCTL_MAGIC, _W1_GPIO_CHAR_IOCTL_CONVERT)
#define W1_GPIO_CHAR_IOCTL_READ_ROM(size)           _IOR(_W1_GPIO_CHAR_IOCTL_MAGIC, _W1_GPIO_CHAR_IOCTL_READ_ROM, size)
#define W1_GPIO_CHAR_IOCTL_READ_DATA_WITH_ROM(size) _IOR(_W1_GPIO_CHAR_IOCTL_MAGIC, _W1_GPIO_CHAR_IOCTL_READ_DATA_WITH_ROM, size)
#define W1_GPIO_CHAR_IOCTL_READ_DATA(size)          _IOR(_W1_GPIO_CHAR_IOCTL_MAGIC, _W1_GPIO_CHAR_IOCTL_READ_DATA, size)

/*---------- type define ----------*/
struct w1_gpio_char_ioctl_arg {
    __u8 rom[9];
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
#endif /* __DRIVER_W1_W1_GPIO_CHAR_IOCTL_H */
