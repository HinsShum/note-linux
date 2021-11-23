/**
 * @file driver\include\d100.h
 *
 * Copyright (C) 2021
 *
 * d100.h is free software: you can redistribute it and/or modify
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
#ifndef __D100_H
#define __D100_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "serial.h"
#include "gpio.h"
#include "pingpong_buffer.h"

/*---------- macro ----------*/
#define D100_PACKAGE_PARITY_SIZE                    (1)
#define D100_PACKAGE_SIZE_MINIMUM                   (offsetof(d100_msg_t, data) + D100_PACKAGE_PARITY_SIZE)
#define D100_PACKAGE_SYNC_WORD                      (0xEFAA)

/* D100 MSG ID defined
 */
#define D100_MSGID_REPLAY                           (0x00)
#define D100_MSGID_NOTE                             (0x01)
#define D100_MSGID_RESET                            (0x10)
#define D100_MSGID_GETSTATUS                        (0x11)
#define D100_MSGID_VERIFY                           (0x12)
#define D100_MSGID_ENROLL                           (0x13)
#define D100_MSGID_SNAPIMAGE                        (0x16)
#define D100_MSGID_GETSAVEDIMAGE                    (0x17)
#define D100_MSGID_UPLOADIMAGE                      (0x18)
#define D100_MSGID_ENROLL_SINGLE                    (0x1D)
#define D100_MSGID_DELUSER                          (0x20)
#define D100_MSGID_DELALL                           (0x21)
#define D100_MSGID_GETUSERINFO                      (0x22)
#define D100_MSGID_FACERESET                        (0x23)
#define D100_MSGID_GET_ALL_USERID                   (0x24)
#define D100_MSGID_ENROLL_TID                       (0x26)
#define D100_MSGID_VERSION                          (0x30)
#define D100_MSGID_START_OTA                        (0x40)
#define D100_MSGID_STOP_OTA                         (0x41)
#define D100_MSGID_GET_OTA_STATUS                   (0x42)
#define D100_MSGID_GET_OTA_HEADER                   (0x43)
#define D100_MSGID_OTA_PACKET                       (0x44)
#define D100_MSGID_INIT_ENCRYPTION                  (0x50)
#define D100_MSGID_CONFIG_BAUDRATE                  (0x51)
#define D100_MSGID_SET_RELEASE_ENC_KEY              (0x52)
#define D100_MSGID_SET_DEBUG_ENC_KEY                (0x53)
#define D100_MSGID_GET_LOGFILE                      (0x60)
#define D100_MSGID_UPLOAD_LOGFILE                   (0x61)
#define D100_MSGID_SET_THRESHOLD_LEVEL              (0xD4)
#define D100_MSGID_POWERDOWN                        (0xED)
#define D100_MSGID_DEBUG_MODE                       (0xF0)
#define D100_MSGID_GET_DEBUG_INFO                   (0xF1)
#define D100_MSGID_UPLOAD_DEBUG_INFO                (0xF2)
#define D100_MSGID_GETLIBRARY_VERSION               (0xF3)
#define D100_MSGID_DEMOMODE                         (0xF4)

/* D100 note id defined
 */
#define D100_NID_READY                              (0x00)
#define D100_NID_FACE_STATE                         (0x01)
#define D100_NID_UNKNOWNERROR                       (0x02)
#define D100_NID_OTA_DONE                           (0x03)
#define D100_NID_EYE_STATE                          (0x04)

/* D100 Reply result defined
 */
#define D100_RES_SUCCESS                            (0x00)
#define D100_RES_REJECTED                           (0x01)
#define D100_RES_ABORTED                            (0x02)
#define D100_RES_FAILED4_CAMERA                     (0x04)
#define D100_RES_FAILED4_UNKNOW                     (0x05)
#define D100_RES_FAILED4_INVALIDPARAM               (0x06)
#define D100_RES_FAILED4_NOMEMORY                   (0x07)
#define D100_RES_FAILED4_UNKNOWUSER                 (0x08)
#define D100_RES_FAILED4_MAXUSER                    (0x09)
#define D100_RES_FAILED4_FACEENROLLED               (0x0A)
#define D100_RES_FAILED4_LIVENESSCHECK              (0x0C)
#define D100_RES_FAILED4_TIMEOUT                    (0x0D)
#define D100_RES_FAILED4_AUTHORIZATION              (0x0E)
#define D100_RES_FAILED4_READ_FILE                  (0x13)
#define D100_RES_FAILED4_WRITE_FILE                 (0x14)
#define D100_RES_FAILED4_NO_ENCRYPT                 (0x15)
#define D100_RES_FAILED4_NO_RGBIMAGE                (0x17)

/* face state defined
 */
#define D100_FACE_STATE_NORMAL                      (0x00)
#define D100_FACE_STATE_NOFACE                      (0x01)
#define D100_FACE_STATE_TOOUP                       (0x02)
#define D100_FACE_STATE_TOODOWN                     (0x03)
#define D100_FACE_STATE_TOOLEFT                     (0x04)
#define D100_FACE_STATE_TOORIGHT                    (0x05)
#define D100_FACE_STATE_FAR                         (0x06)
#define D100_FACE_STATE_CLOSE                       (0x07)
#define D100_FACE_STATE_EYEBROW_OCCLUSION           (0x08)
#define D100_FACE_STATE_EYE_OCCLUSION               (0x09)
#define D100_FACE_STATE_FACE_OCCLUSION              (0x0A)
#define D100_FACE_STATE_DIRECTION_ERROR             (0x0B)
#define D100_FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE   (0x0C)
#define D100_FACE_STATE_EYE_CLOSE_STATUS            (0x0D)
#define D100_FACE_STATE_EYE_CLOSE_UNKNOWN_STATUS    (0x0E)

/* face direction defined
 */
#define D100_FACE_DIRECTION_UP                      (0x10)
#define D100_FACE_DIRECTION_DOWN                    (0x08)
#define D100_FACE_DIRECTION_LEFT                    (0x04)
#define D100_FACE_DIRECTION_RIGHT                   (0x02)
#define D100_FACE_DIRECTION_MIDDLE                  (0x01)
#define D100_FACE_DIRECTION_UNDEFINE                (0x00)

/**
 * @brief Get the serial comport.
 * @note The comport is useful when program running under the windows
 *       or linux platforms. But under the embeded platform, the comport
 *       is not necessary.
 * @para Args is a pointer of the buffer to store the comport information,
 *       the buffer type is `uint8_t`.
 * @retval If the args is is NULL, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_D100_GET_COMPORT                      (IOCTL_SERIAL_GET_COMPORT)

/**
 * @brief Set the irq server callback function.
 * @note If enable the serial interrupt function, when occur once interrupt,
 *       the callback funtion will be called once.
 * @param Args is the pointer of the callback function.
 *        The type is `int32_t (*)(uint32_t irq_handler. void *args, uint32_t length)`.
 * @retval The interface always return CY_EOK.
 */
#define IOCTL_D100_SET_IRQ_HANDLER                  (IOCTL_SERIAL_SET_IRQ_HANDLER)

/**
 * @brief Get the baudrate of the serial device.
 * @param Args is a pointer of the buffer to store the badurate information.
 * @retval If the args is NULL, the interface will return CY_E_WRONG_ARGS,
 *         otherwise, return CY_EOK.
 */
#define IOCTL_D100_GET_BAUDRATE                     (IOCTL_SERIAL_GET_BAUDRATE)

/**
 * @brief Set the baudrate of the serial device.
 * @param Args is a ponter of the baudrate variable address.
 * @retval If the args is NULL, the interface will return CY_E_WRONG_ARGS.
 *         If serial re-initialze failed, the interface will return
 *         CY_ERROR, otherwise, return CY_EOK.
 */
#define IOCTL_D100_SET_BAUDRATE                     (IOCTL_SERIAL_SET_BAUDRATE)

/**
 * @brief Clear the d100 status.
 * @param Args is NULL.
 * @retval The interface will return  CY_EOK.
 */
#define IOCTL_D100_CLEAR_STATUS                     (IOCTL_SERIAL_INHERIT_START + 0x00)

/*---------- type define ----------*/
typedef struct {
    uint16_t sync;
    uint8_t msgid;
    uint8_t size[2];
    uint8_t data[0];
} d100_msg_t;

typedef enum {
    STATE_IDLE,
    STATE_WAIT_RESP,
} d100_state_en_t;

typedef struct {
    uint8_t buf[256];
    uint16_t offset;
    uint16_t expected_length;
} d100_buffer_t;

typedef struct {
    uint8_t msgid;
    uint8_t buf[256];
    uint16_t size;
} d100_read_context_t;

typedef struct {
    uint16_t id;        /*<< heb before, led after */
} d100_user_id_t;

typedef struct {
    uint8_t id_heb;
    uint8_t id_leb;
    uint8_t name[32];
    uint8_t admin;
} d100_user_info_t;

typedef struct {
    uint8_t counts;
    uint8_t data[0];
} d100_all_user_id_t;

typedef struct {
    uint8_t pd_rightaway;   /*<< power down right away after verifying */
    uint8_t timeout;        /*<< timeout, unit second, default 10s */
} d100_verify_t;

typedef struct {
    uint8_t id_heb;
    uint8_t id_leb;
    uint8_t name[32];
    uint8_t admin;
    uint8_t unlock_status;
} d100_reply_verify_t;

typedef struct {
    int16_t state;
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
    int16_t yaw;
    int16_t pitch;
    int16_t roll;
} d100_note_face_state_t;

typedef struct {
    uint8_t admin;
    uint8_t name[32];
    uint8_t face_dir;
    uint8_t timeout;
} d100_enroll_t;

typedef struct {
    uint8_t id_heb;
    uint8_t id_leb;
    uint8_t face_direction;
} d100_reply_enroll_t;

typedef struct {
    serial_describe_t serial;
    gpio_describe_t gpio;
    struct {
        uint8_t msgid;
        d100_state_en_t state;
    } status;
    pingpong_buffer_t pingpong;
    d100_buffer_t buffer1;
    d100_buffer_t buffer2;
    d100_buffer_t *cur_buffer;
} d100_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __D100_H */
