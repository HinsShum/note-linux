/**
 * _nb05_01_at.h
 *
 * Copyright (C) 2018 HinsShum
 *
 * _nb05_01_at.h is free software: you can redistribute it and/or modify
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
#ifndef ___NB05_01_AT_H
#define ___NB05_01_AT_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ryat.h"

/*---------- marco ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t _nbxx_01_at_get_data(struct st_ryat_describe *pat, uint8_t *pdata, uint16_t len, uint16_t *remain, uint32_t timeout);

#endif /* ___NB05_01_AT_H */
