/**
 * /driver/printk.c
 *
 * Copyright (C) 2017 HinsShum
 *
 * printk.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "printk.h"
#include "config/options.h"

#ifdef CONFIG_SYSLOG
#include "ff.h"
#include "diskio.h"
#include "pcf8563.h"
#include "config/attributes.h"
#endif

/*---------- marcos ----------*/
#ifndef PRINTK_LOCK_ENABLE
#define PRINTK_LOCK_ENABLE      (0)
#endif

#if PRINTK_LOCK_ENABLE
    /**
     * FreeRTOS mutex and semaphore
     */
    #define DECLARE_MUTEX(name)         SemaphoreHandle_t name = NULL
    #define mutex_lock_blocked(mutex)   (xSemaphoreTake(mutex, portMAX_DELAY))
    #define mutex_unlock(mutex)         (xSemaphoreGive(mutex))
    #define DECLARE_SEMAPHORE(name)     SemaphoreHandle_t name = NULL
    #define semaphore_up(sem)           (xSemaphoreGive(sem))
    #define semaphore_down(sem)         (xSemaphoreTake(sem, 0))
#else
    #define DECLARE_MUTEX(name)         bool name = true
    #define mutex_lock_blocked(mutex)
    #define mutex_unlock(mutex)
    #define DECLARE_SEMAPHORE(name)     bool __attribute__((unused)) name = true
    #define semaphore_up(sem)    
    #define semaphore_down(sem)         true
#endif

/**
 * log buffer
 */
#ifndef CONFIG_PRINTK_LOG_BUF_SHIFT
#define CONFIG_PRINTK_LOG_BUF_SHIFT     (10)
#endif

#define __LOG_BUF_LEN                   (1 << CONFIG_PRINTK_LOG_BUF_SHIFT)

/**
 * printk's without a loglevel use this..
 */
#define DEFAULT_MESSAGE_LOGLEVEL        (1)     /* KERN_WARNING */
#define DEFAULT_CONSOLE_LOGLEVEL        (4)     /* anything MORE serious than KERN_DEBUG */
#define DEFAULT_SYSLOG_LOGLEVEL         (3)

#define LOG_BUF_MASK        (log_buf_len - 1)
#define LOG_BUF(index)      (log_buf[(index) & LOG_BUF_MASK])

/*---------- variables ----------*/
struct con {
    unsigned int (*write)(const char *, unsigned int);
    bool (*getc)(char *);
};

int console_printk[] = {
    DEFAULT_CONSOLE_LOGLEVEL,   /* console_loglevel */
    DEFAULT_SYSLOG_LOGLEVEL,    /* syslog loglevel */
    DEFAULT_MESSAGE_LOGLEVEL,   /* default_message_loglevel */
};

#ifdef CONFIG_PRINTK
static struct con console_driver;
/**
 * The indices into log_buf are not constrained to log_buf_len - they
 * must be masked before subscripting
 */
static unsigned int log_start = 0;  /* Index into log_buf: next char to be read by syslog() */
static unsigned int con_start = 0;  /* Index into log_buf: next char to be sent to consoles */
static unsigned int log_end = 0;    /* Index into log_buf: most-recently-written-char + 1 */

static char __log_buf[__LOG_BUF_LEN];
static char *log_buf = __log_buf;
static int log_buf_len = __LOG_BUF_LEN;

static DECLARE_MUTEX(logbuf_lock);

static DECLARE_SEMAPHORE(console_sem);

static int new_text_line = 1;
static char printk_buf[__LOG_BUF_LEN];

static char loglevelchar[] = {
    [0] = 'E',  /* ERROR */
    [1] = 'W',  /* WARNING */
    [2] = 'M',  /* MESSAGE */
    [3] = 'I',  /* INFORMATION */
};

#ifdef CONFIG_SYSLOG
static TaskHandle_t syslog_taskhandle;
#endif

static unsigned short classification_level = KERN_CLASSIFICATION_LEVEL_DEFAULT;
/*---------- function ----------*/
static void emit_log_char(char c)
{
    LOG_BUF(log_end) = c;
    log_end++;
    /* log */
    if(log_end - log_start > log_buf_len) {
        log_start = log_end - log_buf_len;
    }
    /* console */
    if(log_end - con_start > log_buf_len) {
        con_start = log_end - log_buf_len;
    }

}

static void wake_up_syslog(void)
{
#ifdef CONFIG_SYSLOG
    xTaskNotifyGive(syslog_taskhandle);
#endif
}

/**
 * try_acquire_console_sem() - Try to get console semaphore
 * without waiting to actually show the kernel messages from a
 * 'printk'.
 *
 * Get the console semaphore return 0, otherwise return -1.
 */
static int try_acquire_console_sem(void)
{
    int retval = -1;

    if(semaphore_down(console_sem)) {
        retval = 0;
    }
    return retval;
}

/**
 * Try to get console ownership to actually show the kernel
 * messages from a 'printk'. Return true (and with the
 * console_semaphore held, and 'console_locked' set) if it
 * is successful, false otherwise.
 *
 * This gets called with the 'logbuf_lock' mutexlock held. It
 * should return with 'lockbuf_lock' released.
 */
static int acquire_console_semaphore_for_printk(void)
{
    int retval = false;

    if(!try_acquire_console_sem()) {
        retval = true;
    }

    mutex_unlock(logbuf_lock);
    return retval;
}

static void __call_console_driver(unsigned int start, unsigned int end)
{
    struct con *con = &console_driver;

    if(con->write) {
        con->write(&LOG_BUF(start), end - start);
    }
}

/**
 * Write out chars from start to end - 1 inclusive
 */
static void _call_console_driever(unsigned int start, unsigned int end, int msg_log_level, int msg_classification_level)
{
    if((msg_classification_level < 0 || (classification_level & (1 << msg_classification_level))) &&
            (msg_log_level < console_loglevel) &&
            (start != end)) {   /*&& console_driver*/
        if((start & LOG_BUF_MASK) > (end & LOG_BUF_MASK)) {
            /* wrapped write */
            __call_console_driver(start & LOG_BUF_MASK, log_buf_len);
            __call_console_driver(0, end & LOG_BUF_MASK);
        } else {
            __call_console_driver(start, end);
        }
    }
}

/**
 * Call the console drivers, asking them to write out
 * log_buf[start] to log_buf[end - 1].
 * The console_sem must be held.
 */
static void call_console_drivers(unsigned int start, unsigned int end)
{
    unsigned int cur_index = 0, start_print = 0;
    static int msg_level = -1, msg_classification_level = -1;

    if(((int)(start - end)) > 0) {
        return ;
    }

    cur_index = start;
    start_print = start;
    while(cur_index != end) {
        if((msg_level < 0) && ((end - cur_index) > 2) &&
                LOG_BUF(cur_index + 0) == '<' &&
                LOG_BUF(cur_index + 1) >= '0' &&
                LOG_BUF(cur_index + 1) <= '3' &&
                LOG_BUF(cur_index + 2) == '>') {
            msg_level = LOG_BUF(cur_index + 1) - '0';
            /**
             * change log level integer to char('E', 'W', 'M', 'I')
             */
            LOG_BUF(cur_index + 1) = loglevelchar[msg_level];
            cur_index += 3;
//          start_print = cur_index;
        }
        if((msg_classification_level < 0) && (end - cur_index) > 2 &&
                LOG_BUF(cur_index + 0) == '[' &&
                LOG_BUF(cur_index + 1) >= '0' &&
                LOG_BUF(cur_index + 1) <= '9' &&
                LOG_BUF(cur_index + 1) >= 'A' &&
                LOG_BUF(cur_index + 1) <= 'B' &&
                LOG_BUF(cur_index + 2) == ']') {
            switch(LOG_BUF(cur_index + 1)) {
                case '0' ... '9' :
                    msg_classification_level = LOG_BUF(cur_index + 1) - '0';
                    break;
                case 'A' ... 'B' :
                    msg_classification_level = LOG_BUF(cur_index + 1) - 'A' + 0x0A;
                    break;
                default : break;
            }
            cur_index += 3;
//          start_print = cur_index;
        }

        while(cur_index != end) {
            char c = LOG_BUF(cur_index);

            cur_index++;
            if(c == '\n') {
                if(msg_level < 0) {
                    /**
                     * printk() has already given us loglevel tags in
                     * the buffer.  This code is here in case the
                     * log buffer has wrapped right round and scribbled
                     * on those tags
                     */
                    msg_level = default_message_loglevel;
                }
                _call_console_driever(start_print, cur_index, msg_level, msg_classification_level);
                msg_level = -1;
                start_print = cur_index;
                break;
            }
        }
    }
    _call_console_driever(start_print, end, msg_level, msg_classification_level);
}

/**
 * release_console_sem() - unlock the console system
 *
 * Releases the semaphore which the caller holds on the console system
 * and the console driver list.
 *
 * While the semaphore was held, console output may have been buffered
 * by printk().  If this is the case, release_console_sem() emits
 * the output prior to releasing the semaphore.
 */
static void release_console_sem(void)
{
    unsigned int _con_start = 0, _log_end = 0;
    unsigned int wake_syslog = 0;

    wake_syslog |= log_end - log_start;
    _con_start = con_start;
    _log_end = log_end;
    con_start = log_end;    /* Flush */

    call_console_drivers(_con_start, _log_end);
    semaphore_up(console_sem);
    if(wake_syslog) {
        /* wake up syslog to read log from ring buffer */
        wake_up_syslog();
    }
}

static inline int vscnprintf(char *buf, unsigned int size, const char *fmt, va_list args)
{
    int len = 0;

    len = vsnprintf(buf, size, fmt, args);
    return ((len >= size) ? (size - 1) : len);
}

static int __attribute__((format(printf, 1, 0))) vprintk(const char *fmt, va_list args)
{
    int printed_len = 0, current_log_level = default_message_loglevel;
    char *p = NULL, current_classification_level = 0;

    /* lock log buf */
    if(!logbuf_lock) {
        return false;
    }
    mutex_lock_blocked(logbuf_lock);
    printed_len = vscnprintf(printk_buf, sizeof(printk_buf), fmt, args);

    p = printk_buf;

    /* Do we have a loglevel in the string? */
    if(p[0] == '<') {
        unsigned char c = p[1];
        if(c && p[2] == '>') {
            switch(c) {
                case '0' ... '3' :  /* loglevel */
                    current_log_level = c - '0';
                /* Fallthrough - make sure we're on a new line */
                case 'd' : /* KERN_DEFAULT */
                    if(!new_text_line) {
                        emit_log_char('\n');
                        printed_len += 1;
                        new_text_line = 1;
                    }
                /* Fallthrough - skip the loglevel */
                case 'c' :
                    p += 3;
                    printed_len -= 3;
                    break;
            }
        }
    }
    /* Do we have a classification level in the string? */
    if(p[0] == '[') {
        unsigned char c = p[1];
        if(c && p[2] == ']') {
            switch(c) {
                case '0' ... '9' :
                case 'A' ... 'B' :
                    current_classification_level = c;
                    p += 3;
                    printed_len -= 3;
                    break;
            }
        }
    }

    /**
     * Copy the output into log_buf.  If the caller didn't provide
     * appropriate log level tags, we insert them here
     */
    for(; *p; p++) {
        if(new_text_line) {
            emit_log_char('<');
            emit_log_char(current_log_level + '0');
            emit_log_char('>');
            printed_len += 3;
            if(current_classification_level) {
                emit_log_char('[');
                emit_log_char(current_classification_level);
                emit_log_char(']');
                printed_len += 3;
            }
            new_text_line = 0;
        }

        emit_log_char(*p);
        if(*p == '\n') {
            new_text_line = 1;
        }
    }
    /**
     * Try to acquire and then immediately release the
     * console semaphore. The release will do all the
     * actual magic (print out buffers, wake up klogd,
     * etc).
     *
     * The acquire_console_semaphore_for_printk() function
     * will release 'logbuf_lock' regardless of whether it
     * actually gets the semaphore or not.
     */
    if(acquire_console_semaphore_for_printk()) {
        release_console_sem();
    }

    return printed_len;
}

/**
 * printk - print a kernel message
 * @fmt: format string
 *
 * This is printk().  It can be called from any context.  We want it to work.
 *
 * We try to grab the console_sem.  If we succeed, it's easy -
 * we log the output and call the console drivers.  If we fail
 * to get the semaphore we place the output into the log buffer
 * and return. The current holder of the console_sem will notice
 * the new output in release_console_sem() and will send it to
 * the consoles before releasing the semaphore.
 *
 * One effect of this deferred printing is that code which calls printk() and
 * then changes console_loglevel may break. This is because console_loglevel
 * is inspected when the actual printing occurs.
 *
 * See also:
 * printf(3)
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */
int printk(const char *fmt, ...)
{
    va_list args;
    int len = 0;

    va_start(args, fmt);
    len = vprintk(fmt, args);
    va_end(args);

    return len;
}

int printk_get_loglevel(unsigned int log)
{
    return (log == CONSOLE_LOG ? console_loglevel : syslog_loglevel);
}

void printk_set_loglevel(unsigned int log, unsigned int loglevel)
{
    int *ploglevel = NULL;

    ploglevel = (log == CONSOLE_LOG ? &console_loglevel : &syslog_loglevel);
    *ploglevel = (loglevel < 4 ? loglevel : 4);
}

unsigned short printk_get_classificationlevel(void)
{
    return classification_level;
}

bool printk_set_classficationlevel(unsigned char classification, bool on)
{
    bool res = false;

    if(classification < 12) {
        if(on) {
            classification_level |= (1 << classification);
        } else {
            classification_level &= (~(1 << classification));
        }
    }

    return res;
}

void printk_open_classficationlevel(void)
{
    classification_level = KERN_CLASSIFICATION_LEVEL_DEFAULT;
}

void printk_close_classficationlevel(void)
{
    classification_level = 0xF000;
}

#ifdef CONFIG_SYSLOG
#include "device.h"
#include "config/errorno.h"

#define VOLUMENAME      "0:"

extern struct st_device dev_pcf8563;

static char filename_buf[12] = { 0 };
static char dirname_buf[12] = { 0 };
static bool syslog_write_flag = false;

DWORD get_fattime(void) {
    extern struct st_device dev_pcf8563;
    time_t log_time = 0;
    struct tm *ptm = NULL;

    if(SL_EOK == device_ioctl(&dev_pcf8563, PCF8563_IOCTL_GET_SECOND, &log_time)) {
        log_time += 8 * 60 * 60;
        ptm = localtime(&log_time);

        return  ((DWORD)(ptm->tm_year + 1900 - 1980) << 25)     /* Year  */
                | ((DWORD)(ptm->tm_mon + 1) << 21)              /* Month */
                | ((DWORD)ptm->tm_mday << 16)                   /* Mday */
                | ((DWORD)ptm->tm_hour << 11)                   /* Hour */
                | ((DWORD)ptm->tm_min << 5)                     /* Min */
                | ((DWORD)ptm->tm_sec >> 1);                    /* Sec */
    } else {
        /* Returns current time packed into a DWORD variable */
        return  ((DWORD)(2013 - 1980) << 25)    /* Year 2013 */
                | ((DWORD)7 << 21)              /* Month 7 */
                | ((DWORD)28 << 16)             /* Mday 28 */
                | ((DWORD)0 << 11)              /* Hour 0 */
                | ((DWORD)0 << 5)               /* Min 0 */
                | ((DWORD)0 >> 1);              /* Sec 0 */
    }
}
/**
 * syslog_write_flag_set() - write syslog to sd card immediately
 *
 * retval: None
 */
void syslog_write_flag_set(void)
{
    syslog_write_flag = true;
}

/**
 * syslog_write_flag_clear() - clear write immediately flag
 *
 * retval: None
 */
void syslog_write_flag_clear(void)
{
    syslog_write_flag = false;
}

/**
 * dir_remove() - Remove dir and files under dir
 * @path: dir pathname
 *
 * retval: fs operate result
 */
static FRESULT dir_remove(char *path)
{
    FRESULT result;
    DIR *dirinfo;
    FILINFO *fileinfo;
    char pathbuf[30] = { 0 };

    dirinfo = sl_malloc(sizeof(DIR));
    if(dirinfo == NULL) {
        return FR_NOT_READY;
    }
    fileinfo = sl_malloc(sizeof(FILINFO));
    if(fileinfo == NULL) {
        sl_free(dirinfo);
        return FR_NOT_READY;
    }

    result = f_opendir(dirinfo, path);
    if(result == FR_OK) {
        for(;;) {
            result = f_readdir(dirinfo, fileinfo);    /* get item one by one */
            if(result != FR_OK) {
                break;
            }
            if(fileinfo->fname[0] == 0) {
                /* get all items */
                break;
            }
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", path, fileinfo->fname);
            if(fileinfo->fattrib & AM_DIR) {
                if(strcmp(fileinfo->fname, ".") && strcmp(fileinfo->fname, "..")) {
                    result = dir_remove(pathbuf);
                    if(result != FR_OK) {
                        printk(KERN_ERROR SYS_SYSLOG_CLASS "Remove dir(%s) failed.\n", pathbuf);
                        break;
                    }
                }
            } else {
                result = f_unlink(pathbuf);
                if(result != FR_OK) {
                    printk(KERN_ERROR SYS_SYSLOG_CLASS "Remove file(%s) failed.\n", pathbuf);
                    break;
                }
            }
        }
        f_closedir(dirinfo);
        result = f_unlink(path);
    }
    sl_free(dirinfo);
    sl_free(fileinfo);

    return result;
}

static FRESULT delete_old_logfile(char *path, struct tm *pnow_time)
{
    FRESULT result;
    DIR *dirinfo;
    FILINFO *fileinfo;
    char pathbuf[20] = { 0 };

    dirinfo = sl_malloc(sizeof(DIR));
    if(dirinfo == NULL) {
        return FR_NOT_READY;
    }
    fileinfo = sl_malloc(sizeof(FILINFO));
    if(fileinfo == NULL) {
        sl_free(dirinfo);
        return FR_NOT_READY;
    }

    result = f_opendir(dirinfo, path);
    if(result == FR_OK) {
        for(;;) {
            result = f_readdir(dirinfo, fileinfo);    /* get item one by one */
            if(result != FR_OK) {
                break;
            }
            if(fileinfo->fname[0] == 0) {
                /* get all items */
                break;
            }
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", path, fileinfo->fname);
            if(fileinfo->fattrib & AM_DIR) {
                if(strcmp(fileinfo->fname, ".") && strcmp(fileinfo->fname, "..")) {
                    int year, month;
                    int tmp = atoi(fileinfo->fname);
                    bool is_remove = false;

                    year = tmp / 100 - 1900;
                    month = tmp % 100 - 1;
                    if((year == pnow_time->tm_year) || ((pnow_time->tm_year - year) == 1)) {
                        if(((pnow_time->tm_mon + 12 - month) % 12) >= 3) {
                            is_remove = true;
                        }
                    } else {
                        is_remove = true;
                    }
                    if(is_remove) {
                        dir_remove(pathbuf);
                    }
                }
            }
        }
        f_closedir(dirinfo);
    }
    sl_free(dirinfo);
    sl_free(fileinfo);

    return result;
}

/**
 * mk_and_cd_dir() - Check dir if exit? if not, create dir and
 * change current dir.
 * @note: The function will check if exit old log file and
 *      delete it.
 * @path: dir pathname
 * @pnow_time: new log file create time
 *
 * retval: true: operate ok, false: operate failed
 */
static bool mk_and_cd_dir(char *path, struct tm *pnow_time)
{
    FRESULT result;
    bool retval = true;
    extern bool platform_get_timesync(void);

    /* Check dir if exit? */
    result = f_stat(path, NULL);
    if(result == FR_NO_FILE) {
        if(platform_get_timesync() == true) {
            /* check old log dir and remove it */
            delete_old_logfile(VOLUMENAME, pnow_time);
        }
        /* create new dir for new log file */
        result = f_mkdir(path);
        if(result != FR_OK) {
            printk(KERN_ERROR SYS_SYSLOG_CLASS "Create dir failed\n");
            retval = false;
        }
    } else if(result == FR_OK) {
        /* pass */
    } else {
        printk(KERN_ERROR SYS_SYSLOG_CLASS "f_stat(%s) An error occured.(%d)\n", path, result);
        retval = false;
    }
    /* cd dir */
    if(retval) {
        result = f_chdir(path);
        if(result != FR_OK) {
            printk(KERN_ERROR SYS_SYSLOG_CLASS "Change current dir failed\n");
            retval = false;
        }
    }

    return retval;
}

static void add_to_file(char *buf, UINT len)
{
    FATFS *fsops;
    FIL *fops;
    FRESULT retval;
    UINT bw;
    struct tm *file_tm = NULL;
    time_t second = 0;
    unsigned char sd_ctl = POWER_ON;

    fsops = sl_malloc(sizeof(FATFS));
    fops = sl_malloc(sizeof(FIL));

    if(unlikely(fsops == NULL)) {
        return ;
    }
    if(unlikely(fops == NULL)) {
        sl_free(fsops);
        return ;
    }
    memset(fsops, 0, sizeof(FATFS));
    memset(fops, 0, sizeof(FIL));
    disk_ioctl(SPI_SD, CTRL_POWER, &sd_ctl);
    retval = f_mount(fsops, VOLUMENAME, 1);
    if(retval != FR_OK) {
        printk(KERN_ERROR SYS_SYSLOG_CLASS "mount SD Card failed!\n");
    } else {
        /**
         * Get real time
         * dirname: year+month(for example --> 201709)
         * filename: day.txt(for example --> 11.txt)
         */
        if(SL_EOK != device_ioctl(&dev_pcf8563, PCF8563_IOCTL_GET_SECOND, &second)) {
            snprintf(filename_buf, sizeof(filename_buf), "%s", "notime.txt");
        } else {
            second += 8 * 60 * 60;
            file_tm = localtime(&second);
            snprintf(dirname_buf, sizeof(dirname_buf), "%04d%02d", file_tm->tm_year + 1900, file_tm->tm_mon + 1);
            snprintf(filename_buf, sizeof(filename_buf), "%02d.txt", file_tm->tm_mday);
            if(true != mk_and_cd_dir(dirname_buf, file_tm)) {
                goto fs_err;
            }
        }

        retval = f_open(fops, filename_buf, FA_WRITE);
        /**
         * Check file if exit?
         */
        if(retval == FR_NO_FILE) {
            retval = f_open(fops, filename_buf, FA_WRITE | FA_CREATE_NEW);
        } else if(retval == FR_OK) {
            f_lseek(fops, f_size(fops));
        } else {
            printk(KERN_ERROR SYS_SYSLOG_CLASS "f_open(%s) failed, INVALID Parameter for f_open()\n", filename_buf);
            goto fs_err;
        }
        if(retval == FR_OK) {
            f_write(fops, buf, len, &bw);
            if(len != bw) {
                printk(KERN_ERROR SYS_SYSLOG_CLASS "write log failed.\n");
            }
            f_close(fops);
        }
    }
fs_err:
    f_mount(NULL, VOLUMENAME, 0);
    vTaskDelay(50);
    sd_ctl = POWER_OFF;
    disk_ioctl(SPI_SD, CTRL_POWER, &sd_ctl);
    sl_free(fsops);
    sl_free(fops);
}

static void task_syslog(void *pvParameters)
{
    uint32_t syslog_notifiedvalue = 0;
    bool syslog_canwrite = false, buffer_full = false;
    char *syslog_buf = NULL, *psyslog_buf = NULL;
    int syslog_msg_loglevel = -1;
    extern struct st_device dev_pcf8563;

    /* malloc memory size for syslog to store temporary message(message level must less than syslog_loglevel) */
    syslog_buf = sl_malloc(_MAX_SS + 24);
    if(unlikely(syslog_buf == NULL)) {
        while(true) {
            printk(KERN_ERROR SYS_SYSLOG_CLASS "No more heap size for syslog to store message.\n");
            vTaskDelay(2000);
        }
    }
    psyslog_buf = syslog_buf;

    while(true) {
        syslog_notifiedvalue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500));
        if(syslog_notifiedvalue && log_start != log_end) {
            struct tm *log_time = NULL;
            time_t now_time = 0;
            bool has_time = false;
            char time_buf[20] = { 0 };

            if(SL_EOK == device_ioctl(&dev_pcf8563, PCF8563_IOCTL_GET_SECOND, &now_time)) {
                has_time = true;
                now_time += 8 * 60 * 60;
                log_time = localtime(&now_time);
            }
            mutex_lock_blocked(logbuf_lock);
            while(log_start != log_end) {
                if(((log_end - log_start) > 2) &&
                        LOG_BUF(log_start + 0) == '<' &&
                        LOG_BUF(log_start + 2) == '>') {
                    switch(LOG_BUF(log_start + 1)) {
                        case 'E' : syslog_msg_loglevel = 0; break;
                        case 'W' : syslog_msg_loglevel = 1; break;
                        case 'M' : syslog_msg_loglevel = 2; break;
                        case 'I' : syslog_msg_loglevel = 3; break;
                    }
                    if(syslog_msg_loglevel < syslog_loglevel) {
                        syslog_canwrite = true;
                        if(has_time) {
                            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", log_time);
                            sprintf(psyslog_buf, "[%s]", time_buf);
                            while(*++psyslog_buf);
                        }
                        *psyslog_buf++ = LOG_BUF(log_start + 0);
                        *psyslog_buf++ = LOG_BUF(log_start + 1);
                        *psyslog_buf++ = LOG_BUF(log_start + 2);
                    } else {
                        syslog_canwrite = false;
                    }
                    log_start += 3;
                }
                while(log_start != log_end) {
                    char c = LOG_BUF(log_start);

                    log_start++;
                    if(syslog_canwrite) {
                        *psyslog_buf++ = c;
                        if(psyslog_buf >= (syslog_buf + _MAX_SS) || syslog_write_flag) {
                            buffer_full = true;
                            syslog_write_flag_clear();
                            break;
                        }
                    }
                    if(c == '\n') {
                        syslog_canwrite = false;
                        break;
                    }
                }
                if(buffer_full) {
                    break;
                }
            }
            mutex_unlock(logbuf_lock);
            if(buffer_full) {
                buffer_full = false;
                add_to_file(syslog_buf, psyslog_buf - syslog_buf);
                /* Flush syslog buffer */
                psyslog_buf = syslog_buf;
            }
        }
    }
}
#endif

/**
 * console_driver_init() - init console
 * @note: if you want to use printk, must initialize console
 *      first.
 *
 * retval: true: console can use, false: console can not use
 */
bool console_driver_init(unsigned int (*write)(const char *, unsigned int))
{
    bool retval = true;

    console_driver.write = write;
#if PRINTK_LOCK_ENABLE
    /**
     * use FreeRTOS as kernel os
     */
    logbuf_lock = xSemaphoreCreateMutex();
    console_sem = xSemaphoreCreateBinary();
    if(!logbuf_lock || !console_sem) {
        retval = false;
    } else {
        semaphore_up(console_sem);
    }
#endif

#ifdef CONFIG_SYSLOG
    /**
     * Create task syslog to read message from ring buffer and
     * write them to file
     * @note: use FreeRTOS as kernel os
     */
    xTaskCreate(task_syslog, "Syslog", SYS_LOG_STACK_SIZE, NULL, SYS_LOG_PRIORITY, &syslog_taskhandle);
#endif

    return retval;
}
#endif
