/**
 * @file atlib/ec20/_ec20_at.h
 *
 * Copyright (C) 2020
 *
 * _slm750_at.h is free software: you can redistribute it and/or modify
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
#ifndef ___EC20_AT_H
#define ___EC20_AT_H

/*---------- includes ----------*/
#include "includes.h"
#include "ryat.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t _ec20_at_send_data(struct st_ryat_describe *pat, ryat_cmd_t *pcmd, uint8_t channel, uint8_t *pdata, uint16_t len);
extern int32_t _ec20_at_get_data(struct st_ryat_describe *pat, uint8_t ch, uint8_t *pdata, uint16_t len, uint32_t timeout);

#endif /* ___EC20_AT_H */