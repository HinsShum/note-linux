/**
 * @file src\inc\ymodem.h
 *
 * Copyright (C) 2021
 *
 * ymodem.h is free software: you can redistribute it and/or modify
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
#ifndef __YMODEM_H
#define __YMODEM_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
#define YMODEM_ERR_OK                       (0)
#define YMODEM_ERR_FAILED                   (-1)
#define YMODEM_ERR_TIMEOUT                  (-2)
#define YMODEM_ERR_INDEX                    (-3)
#define YMODEM_ERR_SAVE                     (-4)
#define YMODEM_ERR_VERIFY                   (-5)
#define YMODEM_ERR_OVER                     (-6)
#define YMODEM_ERR_UNKNOWN                  (-7)

/*---------- type define ----------*/
typedef struct {
    uint8_t *pbuf;
    uint32_t size;
    bool (*get_char)(uint8_t *ch, uint32_t ms);
    bool (*putc)(const uint8_t ch);
    bool (*frame_save)(uint32_t offset, const uint8_t *frame, uint32_t size);
    bool (*file_verify)(const char *filename, uint32_t filesize);
} ymodem_context_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t ymodem_init(ymodem_context_t *ctx);
extern int32_t ymodem_recv_file(void);

#ifdef __cplusplus
}
#endif
#endif /* __YMODEM_H */
