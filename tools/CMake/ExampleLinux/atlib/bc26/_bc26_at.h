/**
 * /atlib/bc26/_bc26_at.h
 *
 * Copyright (C) 2019 HinsShum
 *
 * _bc26_at.h is free software: you can redistribute it and/or modify
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
 */
#ifndef ___BC26_AT_H
#define ___BC26_AT_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "includes.h"
#include "ryat.h"

/*---------- marco ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t _bc26_at_send_data(struct st_ryat_describe *pat, ryat_cmd_t* pcmd, uint8_t channal, uint8_t *pdata, uint16_t len);
extern int32_t _bc26_at_get_data(struct st_ryat_describe *pat, uint8_t channal, uint8_t *pdata, uint16_t len, uint32_t timeout);

#endif /* ___BC26_AT_H */
