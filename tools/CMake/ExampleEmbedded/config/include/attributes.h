/**
 * /config/attributes.h
 *
 * Copyright (C) 2017 HinsShum
 *
 * attributes.h is free software: you can redistribute it and/or modify
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
#ifndef __ATTRIBUTES_H
#define __ATTRIBUTES_H

/*---------- includes ----------*/

/*---------- marco ----------*/
#if  defined ( __GNUC__ )
    #ifndef __weak
        #define __weak   __attribute__((weak))
    #endif /* __weak */
    #ifndef __packed
        #define __packed __attribute__((__packed__))
    #endif /* __packed */

    #define __NOINLINE __attribute__ ( (noinline) )

    #ifndef likely
        #define likely(x)   __builtin_expect(!!(x), 1)
    #endif
    #ifndef unlikely
        #define unlikely(x) __builtin_expect(!!(x), 0)
    #endif
#else
    #ifndef likely
        #define likely(x)   (x)
    #endif
    #ifndef unlikely
        #define unlikely(x) (x)
    #endif
#endif /* __GNUC__ */

#ifndef __noreturn
    #define __noreturn      __attribute__((noreturn))
#endif

#ifndef __used
    #define __used          __attribute__((used))
#endif

#ifndef __unused
    #define __unused        __attribute__((unused))
#endif

#ifndef __section
    #define __section(s)    __attribute__((section(#s)))
#endif

#ifndef __must_check
    #define __must_check    __attribute__((warn_unused_result))
#endif
/*---------- type define ----------*/

/*---------- prototype ----------*/

#endif /* __ATTRIBUTES_H */
