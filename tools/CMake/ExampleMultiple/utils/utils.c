/**
 * @file APP/utils/utils.c
 *
 * Copyright (C) 2021
 *
 * utils.c is free software: you can redistribute it and/or modify
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
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

/*---------- macro ----------*/
#define ishex(in) ((in >= 'a' && in <= 'f') || \
                   (in >= 'A' && in <= 'F') || \
                   (in >= '0' && in <= '9'))

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
unsigned int utils_atoh(char *ap) {
    char *p;
    unsigned int n;
    int digit = 0, lcase = 0;

    p = ap;
    n = 0;
    while (*p == ' ') p++;
    while ((digit = (*p >= '0' && *p <= '9')) != 0 ||
           (lcase = (*p >= 'a' && *p <= 'f')) != 0 ||
           (*p >= 'A' && *p <= 'F')) {
        n *= 16;
        if (digit)	n += *p++ - '0';
        else if (lcase)	n += 10 + (*p++ - 'a');
        else		n += 10 + (*p++ - 'A');
    }
    return (n);
}
int utils_isnum(const char *str) {
    const char   *ptr;
    for (ptr = str; *ptr; ptr++) if (!isdigit((int)*ptr)) return (0);
    return (1);
}

char* utils_strtok_r(char *s, const char *delim, char **save_ptr) {
    char *token;
    if (s == NULL) s = *save_ptr;

    /** Scan leading delimiters.  */
    s += strspn(s, delim);
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }
    /** Find the end of the token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
        /** This token finishes the string.  */
        *save_ptr = token + strlen(token);
    else {
        /** Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}

char* utils_strtok(char *s, const char *delim) {
#if defined(__CC_ARM)
    static char *last;
    return strtok_r(s, delim, &last);
#elif defined(__GNUC__) || defined(__ARMCC_VERSION)
    return strtok(s, delim);
#endif
}

int utils_strtolower(char *s, int len) {
    while ((len--) && (*s != '\0')) {
        if (('A' <= *s) && (*s <= 'Z')) *s += 'a' - 'A';
        s++;
    }
    return len;
}

int utils_strtoupper(char *s, int len) {
    if (strlen(s) < len) {
        len = strlen(s);
    }
    while (len--) {
        if (('a' <= *s) && (*s <= 'z')) *s += 'A' - 'a';
        s++;
    }
    return len;
}

char* utils_strsep(char **stringp, const char *delim) {
    char *s = *stringp;
    char *e;

    if (!s) return NULL;

    e = strpbrk(s, delim);
    if (e) *e++ = '\0';

    *stringp = e;
    return s;
}

char* utils_strnstr(const char *str1, const char *str2, int len) {
    char *s = NULL;
    while (len--) {
        if (!strcmp(s, str1)) {
            return s;
        }
        s++;
    }
    return NULL;
}

int  utils_bitcmp(const char *str1, const char *str2, int len) {
    int i = 0;
    for (i = 0; i < len; i++) {
        if (str1[i] > str2[i]) {
            return 1;
        }
        if (str1[i] < str2[i]) {
            return -1;
        }
    }
    return 0;
}

char* utils_find_split_next(char *str, char split) {
    char *next = NULL;
    while (*str != '\0') {
        if (*str == split) {
            *str = 0x00;
            next = str + 1;
            while (*next == split) {
                next++;
            }
            break;
        }
        str++;
    }
    return next;
}

int utils_nsplit(char *str, char split, int n, char **pout) {
    int i = 0;
    int pre_is_pace = 1;
    if ((NULL == str) || (NULL == pout)) {
        return 0;
    }
    while (*str != '\0') {
        if (i > n) {
            break;
        }
        if ((*str != split) && (pre_is_pace)) {
            pout[i] = str;
            i++;
            pre_is_pace = 0;
        }
        if ((*str == split)) {
            pre_is_pace = 1;
            *str = 0x00;
        }
        str++;
    }
    return i;
}

int utils_nsplit_with_null(char *str, char split, int n, char **pout) {
    int i = 0;
    int pre_is_pace = 1;
    if ((NULL == str) || (NULL == pout)) {
        return 0;
    }
    while (*str != '\0') {
        if (i >= n) {
            break;
        }
        if ((*str != split) && (pre_is_pace)) {
            pout[i] = str;
            pre_is_pace = 0;
        }
        if ((*str == split)) {
            pre_is_pace = 1;
            *str = 0x00;
            i++;
        }
        str++;
    }
    return i;
}

int utils_ishex(int a) {
    return (a >= 'A' && a <= 'F') ||
           (a >= 'a' && a <= 'f') ||
           (a >= '0' && a <= '9');
}

int utils_ishexstr(const char *str) {
    const char   *ptr;
    for (ptr = str; *ptr; ptr++) if (!utils_ishex((int)*ptr)) return (0);
    return (1);
}

int utils_hextoi(int hex) {
    if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    }
    if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;
    } else {
        return hex - '0';
    }
}

int utils_atohb(char *str, char *buffer, int len) {
    int i;
    char  hbits, lbits;

    if (strlen(str) < len * 2) {
        len = strlen(str) >> 1;
    }

    for (i = 0; i < len; i++) {
        //up 4bits
        hbits = 0xff;
        if (*str >= 'A' && *str <= 'F') {
            hbits = *str - 'A' + 10;
        }
        if (*str >= 'a' && *str <= 'f') {
            hbits = *str - 'a' + 10;
        }
        if (*str >= '0' && *str <= '9') {
            hbits = *str - 48;
        }
        if (hbits == 0xff) {
            return 0;
        }
        //low 4bits
        str++;
        lbits = 0xff;
        if (*str >= 'A' && *str <= 'F') {
            lbits = *str - 'A' + 10;
        }
        if (*str >= 'a' && *str <= 'f') {
            lbits = *str - 'a' + 10;
        }
        if (*str >= '0' && *str <= '9') {
            lbits = *str - 48;
        }
        if (lbits == 0xff) {
            return 0;
        }
        str++;
        //
        buffer[i] = (hbits << 4) + lbits;
    }
    return len;
}

static int utils_strcat_get_base_ul(uint32_t *val, uint32_t base) {
    int i = 0;
    while (*val >= base) {
        *val -= base;
        i++;
    }
    if (i > 9) {
        return 9;
    }
    return i;
}

char* utils_strcatul(char *str, uint32_t ul) {
    #if 1
    char *ostr = str+strlen(str);
    uint32_t base = 1000000000ul;
	uint8_t j = 0;
	uint8_t k = 0;
        uint8_t start = false;
	while(base>1){
		j = utils_strcat_get_base_ul(&ul, base);
		if ((j > 0)||(start)){
			ostr[k++] = j + 0x30;
                        start = true;
		}
		base = base / 10;
	}
	//base 1
	ostr[k] = ul + 0x30;
	ostr[k + 1] = 0x00;
        return str;
    #else
    int i= utils_strcat_get_base_ul(&ul,1000000000ul);
    char *str = ostr+strlen(ostr);
    uint8_t j = 0;
    
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    
    i = utils_strcat_get_base_ul(&ul, 100000000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }

    i = utils_strcat_get_base_ul(&ul, 10000000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }

    i = utils_strcat_get_base_ul(&ul, 1000000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    i = utils_strcat_get_base_ul(&ul, 100000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    i = utils_strcat_get_base_ul(&ul, 10000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    i = utils_strcat_get_base_ul(&ul, 1000ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    i = utils_strcat_get_base_ul(&ul, 100ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    i = utils_strcat_get_base_ul(&ul, 10ul);
    if (i > 0) {
        str[j++] = i + 0x30;
    }
    //base 1
    str[j] = ul + 0x30;
    str[j+1] = 0x00;
    return str;
    #endif
}

char* utils_strcatint(char *ostr, uint16_t ul) {
     uint32_t oul = ul;
     return utils_strcatul(ostr,oul);
}  

char *utils_strcathex(char *ostr,uint32_t ul){
    uint8_t i = 0;
    uint8_t tmp = 0;
    char *str = ostr+strlen(ostr);

    for (i = 0;i<8;i++) {
        tmp = (ul&0xF0000000)>>28;
        if (tmp>9) {
            str[i]='A'+(tmp-10);
        }else{
            str[i]='0'+tmp;
        }
        ul<<=4;
    }
    str[i]=0x00;
    return str;
}

static bool utils_is_little_endian(void)
{
    uint32_t t = 0x00000001;
    uint8_t buf[4];

    memcpy(buf, &t, sizeof(buf));

    return (buf[0] == 0x01);
}

uint32_t utils_ntohl(uint32_t nl)
{
    if(utils_is_little_endian()) {
        nl = ((nl & 0x000000FF) << 24) | ((nl & 0x0000FF00) << 8) |\
             ((nl & 0xFF000000) >> 24) | ((nl & 0x00FF0000) >> 8);
    }
    
    return nl;
}

uint16_t utils_ntohs(uint16_t ns)
{
    if(utils_is_little_endian()) {
        ns = ((ns & 0x00FF) << 8) | ((ns & 0xFF00) >> 8);
    }

    return ns;
}

uint32_t utils_htonl(uint32_t hl)
{
    if(utils_is_little_endian()) {
        hl = ((hl & 0x000000FF) << 24) | ((hl & 0x0000FF00) << 8) |\
             ((hl & 0xFF000000) >> 24) | ((hl & 0x00FF0000) >> 8);
    }
    
    return hl;
}

uint16_t utils_htons(uint16_t hs)
{
    if(utils_is_little_endian()) {
        hs = ((hs & 0x00FF) << 8) | ((hs & 0xFF00) >> 8);
    }

    return hs;
}
