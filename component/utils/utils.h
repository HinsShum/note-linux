/**
 * @file APP/utils/include/utils.h
 *
 * Copyright (C) 2021
 *
 * utils.h is free software: you can redistribute it and/or modify
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
#ifndef __UTILS_H
#define __UTILS_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern unsigned int utils_atoh ( char *ap );
extern char * utils_strtok_r (char *s, const char *delim, char **save_ptr);
extern char * utils_strtok(char *s, const char *delim);
extern char * utils_strsep(char **stringp, const char *delim);
extern int utils_strtolower(char *s,int len);
extern int utils_strtoupper(char *s,int len);
extern char *strnstr(const char *str1,const char *str2,int len);
extern int  utils_bitcmp(const char *str1,const char *str2,int len);
extern char *utils_find_split_next(char *str,char split);
extern int utils_nsplit(char *str,char split,int n,char **pout);
extern int utils_nsplit_with_null(char *str, char split, int n, char **pout);
extern int utils_ishex( int a);
extern int utils_hextoi(int hex);
extern int utils_ishexstr(const char *str);
extern int utils_isnum(const char *str);
extern int utils_atohb(char *str, char *buffer, int len);
extern char *utils_strcatul(char *str,uint32_t ul);
extern char *utils_strcatint(char *str,uint16_t ul);
extern char *utils_strcathex(char *str,uint32_t ul);
extern uint32_t utils_ntohl(uint32_t nl);
extern uint16_t utils_ntohs(uint16_t ns);
extern uint32_t utils_htonl(uint32_t hl);
extern uint16_t utils_htons(uint16_t hs);

#endif /* __UTILS_H */
