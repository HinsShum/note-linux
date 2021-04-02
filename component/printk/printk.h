/**
 * /driver/printk.h
 *
 * Copyright (C) 2017 HinsShum
 *
 * printk.h is free software: you can redistribute it and/or modify
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
#ifndef __PRINTK_H
#define __PRINTK_H

/*---------- includes ----------*/
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*---------- marco ----------*/
/**
 * log level
 */
#define KERN_ERROR          "<0>" /* error conditions */
#define KERN_WARN           "<1>" /* warning conditions */
#define KERN_MESSAGE        "<2>" /* message conditions */
#define KERN_INFO           "<3>" /* informational */

/* Use the default kernel loglevel */
#define KERN_DEFAULT        "<d>"

/**
 * Annotation for a "continued" line of log printout (only done after a
 * line that had no enclosing \n). Only to be used by core/arch code
 * during early bootup (a continued line is not SMP-safe otherwise).
 */
#define KERN_CONT           "<c>"

/**
 * log classification
 */
#define KERN_CLASS0         "[0]" /* use as default classification level */
#define KERN_CLASS1         "[1]"
#define KERN_CLASS2         "[2]"
#define KERN_CLASS3         "[3]"
#define KERN_CLASS4         "[4]"
#define KERN_CLASS5         "[5]"
#define KERN_CLASS6         "[6]"
#define KERN_CLASS7         "[7]"
#define KERN_CLASS8         "[8]"
#define KERN_CLASS9         "[9]"
#define KERN_CLASS10        "[A]"
#define KERN_CLASS11        "[B]"

#define KERN_CLASSIFICATION_LEVEL_DEFAULT   (0xFFFF)

extern int console_printk[];

#define console_loglevel            (console_printk[0])
#define syslog_loglevel             (console_printk[1])
#define default_message_loglevel    (console_printk[2])

#define CONSOLE_LOG                 (0)
#define SYS_LOG                     (1)

#ifdef CONFIG_PRINTK
int printk(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
static inline int printk(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static inline int printk(const char *fmt, ...)
{
    return 0;
}

static inline bool console_driver_init(unsigned int (*write)(const char *, unsigned int))
{
    return true;
}

static inline int printk_get_loglevel(unsigned int log)
{
    return 0;
}

static inline void printk_set_loglevel(unsigned int log, unsigned int loglevel)
{
}

static inline unsigned short printk_get_classificationlevel(void)
{
    return 0;
}

static inline bool printk_set_classficationlevel(unsigned char classification, bool on) {
    return true;
}

static inline void printk_open_classficationlevel(void)
{
}

static inline void printk_close_classficationlevel(void)
{
}
#endif

/*---------- type define ----------*/
extern bool console_driver_init(unsigned int (*write)(const char *, unsigned int));
extern int printk_get_loglevel(unsigned int log);
extern void printk_set_loglevel(unsigned int log, unsigned int loglevel);
extern unsigned short printk_get_classificationlevel(void);
extern bool printk_set_classficationlevel(unsigned char classification, bool on);
extern void printk_open_classficationlevel(void);
extern void printk_close_classficationlevel(void);

#ifdef CONFIG_SYSLOG
extern void syslog_write_flag_set(void);
extern void syslog_write_flag_clear(void);
#endif

#endif /* __PRINTK_H */
