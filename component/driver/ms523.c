/**
 * @file driver\ms523.c
 *
 * Copyright (C) 2021
 *
 * ms523.c is free software: you can redistribute it and/or modify
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
#include "ms523.h"
#include "driver.h"
#include "config/errorno.h"
#include "config/options.h"
#include "printk.h"
#include "checksum.h"
#include <string.h>

/*---------- macro ----------*/
#define REG_COMMAND                             (0x01)
#define REG_COMMAND_RCVOFF_POS                  (5)
#define REG_COMMAND_RCVOFF_MSK                  (1 << REG_COMMAND_RCVOFF_POS)
#define REG_COMMAND_RCVOFF                      (REG_COMMAND_RCVOFF_MSK)
#define REG_COMMAND_POWERDOWN_POS               (4)
#define REG_COMMAND_POWERDOWN_MSK               (1 << REG_COMMAND_POWERDOWN_POS)
#define REG_COMMAND_POWERDOWN                   (REG_COMMAND_POWERDOWN_MSK)
#define REG_COMMAND_CMD_POS                     (0)
#define REG_COMMAND_CMD_MSK                     (0xF << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_IDLE                    (0x0 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_MEM                     (0x1 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_GER_RANDOMID            (0x2 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_CALC_CRC                (0x3 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_TRANSMIT                (0x4 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_NO_CHANGE               (0x7 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_RECEIVE                 (0x8 << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_TRANSCEIVE              (0xC << REG_COMMAND_CMD_POS)
#define REG_COMMAND_CMD_SOFTRESET               (0xF << REG_COMMAND_CMD_POS)

#define REG_COM_IRQ                             (0x04)
#define REG_COM_IRQ_SET1_POS                    (7)
#define REG_COM_IRQ_SET1_MSK                    (1 << REG_COM_IRQ_SET1_POS)
#define REG_COM_IRQ_SET1                        (REG_COM_IRQ_SET1_MSK)
#define REG_COM_IRQ_TX_IRQ_POS                  (6)
#define REG_COM_IRQ_TX_IRQ_MSK                  (1 << REG_COM_IRQ_TX_IRQ_POS)
#define REG_COM_IRQ_TX_IRQ                      (REG_COM_IRQ_TX_IRQ_MSK)
#define REG_COM_IRQ_RX_IRQ_POS                  (5)
#define REG_COM_IRQ_RX_IRQ_MSK                  (1 << REG_COM_IRQ_RX_IRQ_POS)
#define REG_COM_IRQ_RX_IRQ                      (REG_COM_IRQ_RX_IRQ_MSK)
#define REG_COM_IRQ_IDLE_IRQ_POS                (4)
#define REG_COM_IRQ_IDLE_IRQ_MSK                (1 << REG_COM_IRQ_IDLE_IRQ_POS)
#define REG_COM_IRQ_IDLE_IRQ                    (REG_COM_IRQ_IDLE_IRQ_MSK)
#define REG_COM_IRQ_HI_ALERT_IRQ_POS            (3)
#define REG_COM_IRQ_HI_ALERT_IRQ_MSK            (1 << REG_COM_IRQ_HI_ALERT_IRQ_POS)
#define REG_COM_IRQ_HI_ALERT_IRQ                (REG_COM_IRQ_HI_ALERT_IRQ_MSK)
#define REG_COM_IRQ_LO_ALERT_IRQ_POS            (2)
#define REG_COM_IRQ_LO_ALERT_IRQ_MSK            (1 << REG_COM_IRQ_LO_ALERT_IRQ_POS)
#define REG_COM_IRQ_LO_ALERT_IRQ                (REG_COM_IRQ_LO_ALERT_IRQ_MSK)
#define REG_COM_IRQ_ERR_IRQ_POS                 (1)
#define REG_COM_IRQ_ERR_IRQ_MSK                 (1 << REG_COM_IRQ_ERR_IRQ_POS)
#define REG_COM_IRQ_ERR_IRQ                     (REG_COM_IRQ_ERR_IRQ_MSK)
#define REG_COM_IRQ_TIMER_IRQ_POS               (0)
#define REG_COM_IRQ_TIMER_IRQ_MSK               (1 << REG_COM_IRQ_TIMER_IRQ_POS)
#define REG_COM_IRQ_TIMER_IRQ                   (REG_COM_IRQ_TIMER_IRQ_MSK)

#define REG_DIVIRQ                              (0x05)
#define REG_DIVIRQ_SET2_POS                     (7)
#define REG_DIVIRQ_SET2_MSK                     (1 << REG_DIVIRQ_SET2_POS)
#define REG_DIVIRQ_SET2                         (REG_DIVIRQ_SET2_MSK)
#define REG_DIVIRQ_TIN_ACT_IRQ_POS              (4)
#define REG_DIVIRQ_TIN_ACT_IRQ_MSK              (1 << REG_DIVIRQ_TIN_ACT_IRQ_POS)
#define REG_DIVIRQ_TIN_ACT_IRQ                  (REG_DIVIRQ_TIN_ACT_IRQ_MSK)
#define REG_DIVIRQ_CRC_IRQ_POS                  (2)
#define REG_DIVIRQ_CRC_IRQ_MSK                  (1 << REG_DIVIRQ_CRC_IRQ_POS)
#define REG_DIVIRQ_CRC_IRQ                      (REG_DIVIRQ_CRC_IRQ_MSK)

#define REG_ERROR                               (0x06)
#define REG_ERROR_WR_ERR_POS                    (7)
#define REG_ERROR_WR_ERR_MSK                    (1 << REG_ERROR_WR_ERR_POS)
#define REG_ERROR_WR_ERR                        (REG_ERROR_WR_ERR_MSK)
#define REG_ERROR_TEMP_ERR_POS                  (6)
#define REG_ERROR_TEMP_ERR_MSK                  (1 << REG_ERROR_TEMP_ERR_POS)
#define REG_ERROR_TEMP_ERR                      (REG_ERROR_TEMP_ERR_MSK)
#define REG_ERROR_BUFFER_OVFL_POS               (4)
#define REG_ERROR_BUFFER_OVFL_MSK               (1 << REG_ERROR_BUFFER_OVFL_POS)
#define REG_ERROR_BUFFER_OVFL                   (REG_ERROR_BUFFER_OVFL_MSK)
#define REG_ERROR_COLL_ERR_POS                  (3)
#define REG_ERROR_COLL_ERR_MSK                  (1 << REG_ERROR_COLL_ERR_POS)
#define REG_ERROR_COLL_ERR                      (REG_ERROR_COLL_ERR_MSK)
#define REG_ERROR_CRC_ERR_POS                   (2)
#define REG_ERROR_CRC_ERR_MSK                   (1 << REG_ERROR_CRC_ERR_POS)
#define REG_ERROR_CRC_ERR                       (REG_ERROR_CRC_ERR_MSK)
#define REG_ERROR_PARITY_ERR_POS                (1)
#define REG_ERROR_PARITY_ERR_MSK                (1 << REG_ERROR_PARITY_ERR_POS)
#define REG_ERROR_PARITY_ERR                    (REG_ERROR_PARITY_ERR_MSK)
#define REG_ERROR_PROTOCOL_ERR_POS              (0)
#define REG_ERROR_PROTOCOL_ERR_MSK              (1 << REG_ERROR_PROTOCOL_ERR_POS)
#define REG_ERROR_PROTOCOL_ERR                  (REG_ERROR_PROTOCOL_ERR_MSK)

#define REG_STATUS2                             (0x08)
#define REG_STATUS2_TEMP_SENS_CLEAR_POS         (7)
#define REG_STATUS2_TEMP_SENS_CLEAR_MSK         (1 << REG_STATUS2_TEMP_SENS_CLEAR_POS)
#define REG_STATUS2_TEMP_SENS_CLEAR             (REG_STATUS2_TEMP_SENS_CLEAR_MSK)
#define REG_STATUS2_I2C_FORCE_HS_POS            (6)
#define REG_STATUS2_I2C_FORCE_HS_MSK            (1 << REG_STATUS_I2C_FORCE_HS_POS)
#define REG_STATUS2_I2C_FORCE_HS                (REG_STATUS_I2C_FORCE_HS_MSK)
#define REG_STATUS2_CRYPTO10N_POS               (3)
#define REG_STATUS2_CRYPTO10N_MSK               (1 << REG_STATUS2_CRYPTO10N_POS)
#define REG_STATUS2_CRYPTO10N                   (REG_STATUS2_CRYPTO10N_MSK)
#define REG_STATUS2_MODEMSTATE_POS              (0)
#define REG_STATUS2_MODEMSTATE_MSK              (0x7 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_IDLE             (0x0 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_WAIT_STARTSEND   (0x1 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_TX_WAIT          (0x2 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_TX               (0x3 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_RX_WAIT          (0x4 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_WAIT_DATA        (0x5 << REG_STATUS2_MODEMSTATE_POS)
#define REG_STATUS2_MODEMSTATE_RX               (0x6 << REG_STATUS2_MODEMSTATE_POS)

#define REG_FIFO_DATA                           (0x09)

#define REG_FIFO_LEVEL                          (0x0A)
#define REG_FIFO_LEVEL_FLUSH_BUFFER_POS         (7)
#define REG_FIFO_LEVEL_FLUSH_BUFFER_MSK         (1 << REG_FIFO_LEVEL_FLUSH_BUFFER_POS)
#define REG_FIFO_LEVEL_FLUSH_BUFFER             (REG_FIFO_LEVEL_FLUSH_BUFFER_MSK)
#define REG_FIFO_LEVEL_DATA_NUMBERS_POS         (0)
#define REG_FIFO_LEVEL_DATA_NUMBERS_MSK         (0x7F << REG_FIFO_LEVEL_DATA_NUMBERS_POS)

#define REG_CONTROL                             (0x0C)
#define REG_CONTROL_TSTOP_NOW_POS               (7)
#define REG_CONTROL_TSTOP_NOW_MSK               (1 << REG_CONTROL_TSTOP_NOW_POS)
#define REG_CONTROL_TSTOP_NOW                   (REG_CONTROL_TSTOP_NOW_MSK)
#define REG_CONTROL_TSTART_NOW_POS              (6)
#define REG_CONTROL_TSTART_NOW_MSK              (1 << REG_CONTROL_TSTART_NOW_POS)
#define REG_CONTROL_TSTART_NOW                  (REG_CONTROL_TSTART_NOW_MSK)
#define REG_CONTROL_RX_LAST_BITS_POS            (0)
#define REG_CONTROL_RX_LAST_BITS_MSK            (0x7 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_ALL            (0x0 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_1              (0x1 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_2              (0x2 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_3              (0x3 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_4              (0x4 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_5              (0x5 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_6              (0x6 << REG_CONTROL_RX_LAST_BITS_POS)
#define REG_CONTROL_RX_LAST_BITS_7              (0x7 << REG_CONTROL_RX_LAST_BITS_POS)

#define REG_BIT_FRAMING                         (0x0D)
#define REG_BIT_FRAMING_START_SEND_POS          (7)
#define REG_BIT_FRAMING_START_SEND_MSK          (1 << REG_BIT_FRAMING_START_SEND_POS)
#define REG_BIT_FRAMING_START_SEND              (REG_BIT_FRAMING_START_SEND_MSK)
#define REG_BIT_FRAMING_RX_ALIGN_POS            (4)
#define REG_BIT_FRAMING_RX_ALIGN_MSK            (0x7 << REG_BIT_FRAMING_RX_ALIGN_POS)
#define REG_BIT_FRAMING_RX_ALIGN_0              (0x0 << REG_BIT_FRAMING_RX_ALIGN_POS)
#define REG_BIT_FRAMING_RX_ALIGN_1              (0x1 << REG_BIT_FRAMING_RX_ALIGN_POS)
#define REG_BIT_FRAMING_RX_ALIGN_7              (0x7 << REG_BIT_FRAMING_RX_ALIGN_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_POS        (0)
#define REG_BIT_FRAMING_TX_LAST_BITS_MSK        (0x7 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_ALL        (0x0 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_1          (0x1 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_2          (0x2 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_3          (0x3 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_4          (0x4 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_5          (0x5 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_6          (0x6 << REG_BIT_FRAMING_TX_LAST_BITS_POS)
#define REG_BIT_FRAMING_TX_LAST_BITS_7          (0x7 << REG_BIT_FRAMING_TX_LAST_BITS_POS)

#define REG_COLL                                (0x0E)
#define REG_COLL_VALUES_AFTER_COLL_POS          (7)
#define REG_COLL_VALUES_AFTER_COLL_MSK          (1 << REG_COLL_VALUES_AFTER_COLL_POS)
#define REG_COLL_VALUES_AFTER_COLL              (REG_COLL_VALUES_AFTER_COLL_MSK)
#define REG_COLL_POST_NOT_VALID_POS             (5)
#define REG_COLL_POST_NOT_VALID_MSK             (1 << REG_COLL_POST_NOT_VALID_POS)
#define REG_COLL_POST_NOT_VALID                 (REG_COLL_POST_NOT_VALID_MSK)
#define REG_COLL_COLLPOS_POS                    (0)
#define REG_COLL_COLLPOS_MSK                    (0x1F << REG_COLL_COLLPOS_POS)
#define REG_COLL_COLLPOS_32                     (0x00 << REG_COLL_COLLPOS_POS)
#define REG_COLL_COLLPOS_1                      (0x01 << REG_COLL_COLLPOS_POS)
#define REG_COLL_COLLPOS_8                      (0x08 << REG_COLL_COLLPOS_POS)

#define REG_MODE                                (0x11)
#define REG_MODE_MSB_FIRST_POS                  (7)
#define REG_MODE_MSB_FIRST_MSK                  (1 << REG_MODE_MSB_FIRST_POS)
#define REG_MODE_MSB_FIRST                      (REG_MODE_MSB_FIRST_MSK)
#define REG_MODE_TX_WAIT_RF_POS                 (5)
#define REG_MODE_TX_WAIT_RF_MSK                 (1 << REG_MODE_TX_WAIT_RF_POS)
#define REG_MODE_TX_WAIT_RF                     (REG_MODE_TX_WAIT_RF_MSK)
#define REG_MODE_POL_TIN_POS                    (3)
#define REG_MODE_POL_TIN_MSK                    (1 << REG_MODE_POL_TIN_POS)
#define REG_MODE_POL_TIN                        (REG_MODE_POL_TIN_MSK)
#define REG_MODE_CRC_PRESET_POS                 (0)
#define REG_MODE_CRC_PRESET_MSK                 (0x3 << REG_MODE_CRC_PRESET_POS)
#define REG_MODE_CRC_PRESET_0000                (0x0 << REG_MODE_CRC_PRESET_POS)
#define REG_MODE_CRC_PRESET_6363                (0x1 << REG_MODE_CRC_PRESET_POS)
#define REG_MODE_CRC_PRESET_A671                (0x2 << REG_MODE_CRC_PRESET_POS)
#define REG_MODE_CRC_PRESET_FFFF                (0x3 << REG_MODE_CRC_PRESET_POS)

#define REG_TX_CONTROL                          (0x14)
#define REG_TX_CONTROL_INV_TX_2RF_ON_POS        (7)
#define REG_TX_CONTROL_INV_TX_2RF_ON_MSK        (1 << REG_TX_CONTROL_INV_TX_2RF_ON_POS)
#define REG_TX_CONTROL_INV_TX_2RF_ON            (REG_TX_CONTROL_INV_TX_2RF_ON_MSK)
#define REG_TX_CONTROL_INV_TX_1RF_ON_POS        (6)
#define REG_TX_CONTROL_INV_TX_1RF_ON_MSK        (1 << REG_TX_CONTROL_INV_TX_1RF_ON_POS)
#define REG_TX_CONTROL_INV_TX_1RF_ON            (REG_TX_CONTROL_INV_TX_1RF_ON_MSK)
#define REG_TX_CONTROL_INV_TX_2RF_OFF_POS       (5)
#define REG_TX_CONTROL_INV_TX_2RF_OFF_MSK       (1 << REG_TX_CONTROL_INV_TX_2RF_OFF_POS)
#define REG_TX_CONTROL_INV_TX_2RF_OFF           (REG_TX_CONTROL_INV_TX_2RF_OFF_MSK)
#define REG_TX_CONTROL_INV_TX_1RF_OFF_POS       (4)
#define REG_TX_CONTROL_INV_TX_1RF_OFF_MSK       (1 << REG_TX_CONTROL_INV_TX_1RF_OFF_POS)
#define REG_TX_CONTROL_INV_TX_1RF_OFF           (REG_TX_CONTROL_INV_TX_1RF_OFF_MSK)
#define REG_TX_CONTROL_TX_2CW_POS               (3)
#define REG_TX_CONTROL_TX_2CW_MSK               (1 << REG_TX_CONTROL_TX_2CW_POS)
#define REG_TX_CONTROL_TX_2CW_NOT_MODULATE      (1 << REG_TX_CONTROL_TX_2CW_POS)
#define REG_TX_CONTROL_TX_2CW_MODULATE          (0 << REG_TX_CONTROL_TX_2CW_POS)
#define REG_TX_CONTROL_TX_2RF_ENABLE_POS        (1)
#define REG_TX_CONTROL_TX_2RF_ENABLE_MSK        (1 << REG_TX_CONTROL_TX_2RF_ENABLE_POS)
#define REG_TX_CONTROL_TX_2RF_ENABLE            (REG_TX_CONTROL_TX_2RF_ENABLE_MSK)
#define REG_TX_CONTROL_TX_1RF_ENABLE_POS        (0)
#define REG_TX_CONTROL_TX_1RF_ENABLE_MSK        (1 << REG_TX_CONTROL_TX_1RF_ENABLE_POS)
#define REG_TX_CONTROL_TX_1RF_ENABLE            (REG_TX_CONTROL_TX_1RF_ENABLE_MSK)

#define REG_TXASK                               (0x15)
#define REG_TXASK_FORCE_100ASK_POS              (6)
#define REG_TXASK_FORCE_100ASK_MSK              (1 << REG_TXASK_FORCE_100ASK_POS)
#define REG_TXASK_FORCE_100ASK                  (REG_TXASK_FORCE_100ASK_MSK)

#define REG_RX_SEL                              (0x17)
#define REG_RX_SEL_UART_SEL_POS                 (6)
#define REG_RX_SEL_UART_SEL_MSK                 (0x3 << REG_RX_SEL_UART_SEL_POS)
#define REG_RX_SEL_UART_SEL_LOW                 (0x0 << REG_RX_SEL_UART_SEL_POS)
#define REG_RX_SEL_UART_SEL_TIN_MANCHESTER      (0x1 << REG_RX_SEL_UART_SEL_POS)
#define REG_RX_SEL_UART_SEL_ANALOG              (0x2 << REG_RX_SEL_UART_SEL_POS)
#define REG_RX_SEL_UART_SEL_TIN_NRZ             (0x3 << REG_RX_SEL_UART_SEL_POS)
#define REG_RX_SEL_RX_WAIT_POS                  (0)
#define REG_RX_SEL_RX_WAIT_MSK                  (0x3F << REG_RX_SEL_RX_WAIT_POS)

#define REG_CRC_RESULT_H                        (0x21)
#define REG_CRC_RESULT_L                        (0x22)

#define REG_RF_CONFIG                           (0x26)
#define REG_RF_CONFIG_RX_GAIN_POS               (4)
#define REG_RF_CONFIG_RX_GAIN_MSK               (0x7 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_18DB              (0x0 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_23DB              (0x1 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_33DB              (0x4 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_38DB              (0x5 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_43DB              (0x6 << REG_RF_CONFIG_RX_GAIN_POS)
#define REG_RF_CONFIG_RX_GAIN_48DB              (0x7 << REG_RF_CONFIG_RX_GAIN_POS)

#define REG_TMODE                               (0x2A)
#define REG_TMODE_TAUTO_POS                     (7)
#define REG_TMODE_TAUTO_MSK                     (1 << REG_TMODE_TAUTO_POS)
#define REG_TMODE_TAUTO                         (REG_TMODE_TAUTO_MSK)
#define REG_TMODE_TGATED_POS                    (5)
#define REG_TMODE_TGATED_MSK                    (0x3 << REG_TMODE_TGATED_POS)
#define REG_TMODE_TGATED_NO                     (0x0 << REG_TMODE_TGATED_POS)
#define REG_TMODE_TGATED_TIN                    (0x1 << REG_TMODE_TGATED_POS)
#define REG_TMODE_TGATED_AUX1                   (0x2 << REG_TMODE_TGATED_POS)
#define REG_TMODE_TAUTO_RESTART_POS             (4)
#define REG_TMODE_TAUTO_RESTART_MSK             (1 << REG_TMODE_TAUTO_RESTART_POS)
#define REG_TMODE_TAUTO_RESTART                 (REG_TMODE_TAUTO_RESTART_MSK)
#define REG_TMODE_TPRESCALER_H_POS              (0)
#define REG_TMODE_TPRESCALER_H_MSK              (0xF << REG_TMODE_TPRESCALER_H_POS)

#define REG_TPRESCALER_L                        (0x2B)
#define REG_TRELOAD_H                           (0x2C)
#define REG_TRELOAD_L                           (0x2D)

#define REG_VERSION                             (0x37)

/* PICC command
 */
#define PICC_COMMAND_HALT                       (0x50)
#define PICC_COMMAND_REQUEST_IDLE               (0x26)
#define PICC_COMMAND_REQUEST_ALL                (0x52)
#define PICC_COMMAND_ANTICOLL1                  (0X93)
#define PICC_COMMAND_ANTICOLL2                  (0x95)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t ms523_open(driver_t **pdrv);
static void ms523_close(driver_t **pdrv);
static int32_t ms523_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

/*---------- type define ----------*/
typedef enum {
    MS523_CARD_TYPE_ISO14443_A,
    MS523_CARD_TYPE_ISO14443_B
} ms523_card_type_en;

typedef enum {
    MS523_PCD_REQUEST_IDLE = PICC_COMMAND_REQUEST_IDLE,
    MS523_PCD_REQUEST_ALL = PICC_COMMAND_REQUEST_ALL
} ms523_pcd_request_type_en;

/*---------- variable ----------*/
DRIVER_DEFINED(ms523, ms523_open, ms523_close, NULL, NULL, ms523_ioctl, NULL);

/*---------- function ----------*/
static void ms523_write_reg(ms523_describe_t *pdesc, uint8_t reg, uint8_t value)
{
    reg = (reg << 1) & 0x7E;
    pdesc->cs_ctrl(true);
    pdesc->xfer(reg);
    pdesc->xfer(value);
    pdesc->cs_ctrl(false);
}

static uint8_t ms523_read_reg(ms523_describe_t *pdesc, uint8_t reg)
{
    uint8_t value = 0;

    reg = ((reg << 1) & 0x7E) | 0x80;
    pdesc->cs_ctrl(true);
    pdesc->xfer(reg);
    value = pdesc->xfer(0x00);
    pdesc->cs_ctrl(false);

    return value;
}

static void ms523_reset(ms523_describe_t *pdesc)
{
    pdesc->reset_ctrl(true);
    __delay_us(10);
    pdesc->reset_ctrl(false);
    __delay_us(100);
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_SOFTRESET);
    while(REG_COMMAND_POWERDOWN == (ms523_read_reg(pdesc, REG_COMMAND) & REG_COMMAND_POWERDOWN_MSK));
}

static void ms523_pcd_config_iso_type(ms523_describe_t *pdesc, ms523_card_type_en type)
{
    uint8_t value = 0;

    if(MS523_CARD_TYPE_ISO14443_A == type) {
        /* close cryto10n */
        value = ms523_read_reg(pdesc, REG_STATUS2);
        value &= ~REG_STATUS2_CRYPTO10N_MSK;
        ms523_write_reg(pdesc, REG_STATUS2, value);
        /* configure mode */
        value = ms523_read_reg(pdesc, REG_MODE);
        value &= ~(REG_MODE_TX_WAIT_RF_MSK | REG_MODE_POL_TIN_MSK | REG_MODE_CRC_PRESET_MSK);
        value |= (REG_MODE_TX_WAIT_RF | REG_MODE_POL_TIN | REG_MODE_CRC_PRESET_6363);
        ms523_write_reg(pdesc, REG_MODE, value);
        /* configure timer */
        ms523_write_reg(pdesc, REG_TRELOAD_L, 30);
        ms523_write_reg(pdesc, REG_TRELOAD_H, 0);
        value = REG_TMODE_TAUTO | 0x0D;
        ms523_write_reg(pdesc, REG_TMODE, value);
        ms523_write_reg(pdesc, REG_TPRESCALER_L, 0x3E);
        /* configure modulation */
        value = ms523_read_reg(pdesc, REG_TXASK);
        value |= REG_TXASK_FORCE_100ASK;
        ms523_write_reg(pdesc, REG_TXASK, value);
        /* configure rf */
        value = REG_RX_SEL_UART_SEL_ANALOG | (0x06 & REG_RX_SEL_RX_WAIT_MSK);
        ms523_write_reg(pdesc, REG_RX_SEL, value);
        value = ms523_read_reg(pdesc, REG_RF_CONFIG);
        value &= ~REG_RF_CONFIG_RX_GAIN_MSK;
        value |= REG_RF_CONFIG_RX_GAIN_48DB;
        ms523_write_reg(pdesc, REG_RF_CONFIG, value);
    }
}

static void ms523_pcd_antenna_ctrl(ms523_describe_t *pdesc, bool ctrl)
{
    uint8_t value = 0;

    value = ms523_read_reg(pdesc, REG_TX_CONTROL);
    value &= ~(REG_TX_CONTROL_TX_2RF_ENABLE_MSK | REG_TX_CONTROL_TX_1RF_ENABLE_MSK);
    if(ctrl) {
        value |= REG_TX_CONTROL_TX_2RF_ENABLE | REG_TX_CONTROL_TX_1RF_ENABLE;
    }
    ms523_write_reg(pdesc, REG_TX_CONTROL, value);
}

static int32_t ms523_open(driver_t **pdrv)
{
    ms523_describe_t *pdesc = NULL;
    int32_t retval = CY_EOK;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->init) {
        if(pdesc->init()) {
            ms523_reset(pdesc);
            printk(KERN_INFO "MS523 version: %02X\n", ms523_read_reg(pdesc, REG_VERSION));
            ms523_pcd_config_iso_type(pdesc, MS523_CARD_TYPE_ISO14443_A);
            ms523_pcd_antenna_ctrl(pdesc, false);
            __delay_ms(100);
            ms523_pcd_antenna_ctrl(pdesc, true);
        } else {
            retval = CY_ERROR;
            printk(KERN_ERROR "MS523 bsp initialize failed\n");
        }
    }

    return retval;
}

static void ms523_close(driver_t **pdrv)
{
    ms523_describe_t *pdesc = NULL;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    if(pdesc && pdesc->deinit) {
        pdesc->deinit();
    }
}

uint16_t ms523_crc16(ms523_describe_t *pdesc, uint8_t *pdata, uint16_t len)
{
    uint8_t value = 0;
    uint16_t crc16 = 0;

    /* clear crc irq flag */
    value = ms523_read_reg(pdesc, REG_DIVIRQ);
    value &= ~REG_DIVIRQ_CRC_IRQ_MSK;
    ms523_write_reg(pdesc, REG_DIVIRQ, value);
    /* enter idle */
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_IDLE);
    /* flush fifo */
    value = ms523_read_reg(pdesc, REG_FIFO_LEVEL);
    value |= REG_FIFO_LEVEL_FLUSH_BUFFER;
    ms523_write_reg(pdesc, REG_FIFO_LEVEL, value);
    /* write buf to fifo */
    for(uint16_t i = 0; i < len; ++i) {
        ms523_write_reg(pdesc, REG_FIFO_DATA, pdata[i]);
    }
    /* start crc */
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_CALC_CRC);
    /* get crc flag */
    do {
        value = ms523_read_reg(pdesc, REG_DIVIRQ);
    } while((value & REG_DIVIRQ_CRC_IRQ_MSK) != REG_DIVIRQ_CRC_IRQ);
    /* get crc result */
    crc16 = ms523_read_reg(pdesc, REG_CRC_RESULT_H);
    crc16 <<= 8;
    crc16 |= ms523_read_reg(pdesc, REG_CRC_RESULT_L);

    return crc16;
}

static int32_t ms523_pcd_write_cmd(ms523_describe_t *pdesc, uint8_t *pdata, uint16_t len, uint8_t *pout, uint16_t max_len)
{
    uint8_t value = 0;
    uint8_t timeout_ms = 25;
    int32_t retval = CY_EOK;

    /* clear all irq flag */
    value = ms523_read_reg(pdesc, REG_COM_IRQ);
    value &= ~REG_COM_IRQ_SET1_MSK;
    ms523_write_reg(pdesc, REG_COM_IRQ, value);
    /* enter idle */
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_IDLE);
    /* flush fifo */
    value = ms523_read_reg(pdesc, REG_FIFO_LEVEL);
    value |= REG_FIFO_LEVEL_FLUSH_BUFFER;
    ms523_write_reg(pdesc, REG_FIFO_LEVEL, value);
    /* write buf to fifo */
    for(uint16_t i = 0; i < len; ++i) {
        ms523_write_reg(pdesc, REG_FIFO_DATA, pdata[i]);
    }
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_TRANSCEIVE);
    value = ms523_read_reg(pdesc, REG_BIT_FRAMING);
    value |= REG_BIT_FRAMING_START_SEND;
    ms523_write_reg(pdesc, REG_BIT_FRAMING, value);
    /* get irq flag */
    do {
        do {
            value = ms523_read_reg(pdesc, REG_COM_IRQ);
            if(value & (REG_COM_IRQ_RX_IRQ_MSK | REG_COM_IRQ_IDLE_IRQ)) {
                break;
            }
            if(value & REG_COM_IRQ_TIMER_IRQ_MSK) {
                timeout_ms = 0;
                break;
            }
            timeout_ms--;
            __delay_ms(1);
        } while(timeout_ms > 0);
        value = ms523_read_reg(pdesc, REG_BIT_FRAMING);
        value &= ~REG_BIT_FRAMING_START_SEND_MSK;
        ms523_write_reg(pdesc, REG_BIT_FRAMING, value);
        if(!timeout_ms) {
            retval = CY_E_TIME_OUT;
            break;
        }
        value = ms523_read_reg(pdesc, REG_ERROR);
        if(value & (REG_ERROR_BUFFER_OVFL_MSK | REG_ERROR_COLL_ERR_MSK | REG_ERROR_PARITY_ERR_MSK | REG_ERROR_PROTOCOL_ERR_MSK)) {
            retval = CY_ERROR;
            break;
        }
        value = ms523_read_reg(pdesc, REG_FIFO_LEVEL);
        retval = (int32_t)value;
        if(max_len > value) {
            max_len = value;
        }
        if(!max_len) {
            break;
        }
        value = ms523_read_reg(pdesc, REG_CONTROL);
        value &= 0x07;
        retval = (retval - 1) * 8;
        retval = (value ? retval + value : retval + 8);
        for(uint16_t i = 0; i < max_len; ++i) {
            pout[i] = ms523_read_reg(pdesc, REG_FIFO_DATA);
        }
    } while(0);
    /* stop timer */
    value = ms523_read_reg(pdesc, REG_CONTROL);
    value |= REG_CONTROL_TSTOP_NOW;
    ms523_write_reg(pdesc, REG_CONTROL, value);
    /* enter idle */
    ms523_write_reg(pdesc, REG_COMMAND, REG_COMMAND_CMD_IDLE);

    return retval;
}

static void ms523_pcd_halt(ms523_describe_t *pdesc)
{
    uint8_t buf[18] = {0};
    uint16_t crc16 = 0;

    buf[0] = PICC_COMMAND_HALT;
    buf[1] = 0x00;
    crc16 = ms523_crc16(pdesc, buf, 2);
    buf[2] = crc16 & 0xFF;
    buf[3] = (crc16 >> 8) & 0xFF;
    ms523_pcd_write_cmd(pdesc, buf, 4, buf, ARRAY_SIZE(buf));
}

static int32_t ms523_pcd_request(ms523_describe_t *pdesc, ms523_pcd_request_type_en type, uint8_t **pbuf)
{
    uint8_t value = 0;
    uint8_t buf[18] = {0};
    int32_t retval = CY_EOK;

    /* clear cryto10n */
    value = ms523_read_reg(pdesc, REG_STATUS2);
    value &= ~REG_STATUS2_CRYPTO10N_MSK;
    ms523_write_reg(pdesc, REG_STATUS2, value);
    /* tx last bits */
    ms523_write_reg(pdesc, REG_BIT_FRAMING, REG_BIT_FRAMING_TX_LAST_BITS_7);
    /* make command */
    buf[0] = type;
    retval = ms523_pcd_write_cmd(pdesc, buf, 1, buf, ARRAY_SIZE(buf));
    if(CY_EOK < retval && 0x10 == retval) {
        memcpy(pdesc->card_type, buf, ARRAY_SIZE(pdesc->card_type));
        *pbuf = pdesc->card_type;
        retval = CY_EOK;
    } else {
        retval = CY_ERROR;
    }

    return retval;
}

static int32_t ms523_pcd_anticoll(ms523_describe_t *pdesc, uint8_t **pbuf)
{
    uint8_t value = 0;
    int32_t retval = CY_EOK;
    uint8_t buf[18] = {0};

    /* clear cryto10n */
    value = ms523_read_reg(pdesc, REG_STATUS2);
    value &= ~REG_STATUS2_CRYPTO10N_MSK;
    ms523_write_reg(pdesc, REG_STATUS2, value);
    /* tx last bits */
    ms523_write_reg(pdesc, REG_BIT_FRAMING, REG_BIT_FRAMING_TX_LAST_BITS_ALL);
    /* set coll */
    value = ms523_read_reg(pdesc, REG_COLL);
    value &= ~REG_COLL_VALUES_AFTER_COLL_MSK;
    ms523_write_reg(pdesc, REG_COLL, value);
    /* make command */
    buf[0] = PICC_COMMAND_ANTICOLL1;
    buf[1] = 0x20;
    retval = ms523_pcd_write_cmd(pdesc, buf, 2, buf, ARRAY_SIZE(buf));
    if(CY_EOK < retval && 40 == retval) {
        /* check card id */
        if(buf[4] == checksum_xor(buf, 4)) {
            memcpy(pdesc->card_id, buf, ARRAY_SIZE(pdesc->card_id));
            *pbuf = pdesc->card_id;
            retval = CY_EOK;
        } else {
            retval = CY_ERROR;
        }
    }

    return retval;
}

static int32_t ms523_pcd_select(ms523_describe_t *pdesc)
{
    uint8_t value = 0;
    int32_t retval = CY_EOK;
    uint8_t buf[18] = {0};
    uint16_t crc16 = 0;

    /* clear cryto10n */
    value = ms523_read_reg(pdesc, REG_STATUS2);
    value &= ~REG_STATUS2_CRYPTO10N_MSK;
    ms523_write_reg(pdesc, REG_STATUS2, value);
    /* make command */
    buf[0] = PICC_COMMAND_ANTICOLL1;
    buf[1] = 0x70;
    memcpy(&buf[2], pdesc->card_id, ARRAY_SIZE(pdesc->card_id));
    buf[6] = checksum_xor(pdesc->card_id, ARRAY_SIZE(pdesc->card_id));
    crc16 = ms523_crc16(pdesc, buf, 7);
    buf[7] = crc16 & 0xFF;
    buf[8] = (crc16 >> 8) & 0xFF;
    retval = ms523_pcd_write_cmd(pdesc, buf, 9, buf, ARRAY_SIZE(buf));
    if(CY_EOK < retval && 0x18 == retval) {
        retval = CY_EOK;
    } else {
        retval = CY_ERROR;
    }

    return retval;
}

static int32_t ms523_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    ms523_describe_t *pdesc = NULL;
    int32_t retval = CY_E_WRONG_ARGS;

    assert(pdrv);
    pdesc = container_of((void **)pdrv, device_t, pdrv)->pdesc;
    switch(cmd) {
        case IOCTL_MS523_PCD_HALT:
            if(pdesc) {
                ms523_pcd_halt(pdesc);
                retval = CY_EOK;
            }
            break;
        case IOCTL_MS523_PCD_REQUEST_IDLE:
            if(pdesc && args) {
                retval = ms523_pcd_request(pdesc, MS523_PCD_REQUEST_IDLE, (uint8_t **)args);
            }
            break;
        case IOCTL_MS523_PCD_REQUEST_ALL:
            if(pdesc && args) {
                retval = ms523_pcd_request(pdesc, MS523_PCD_REQUEST_ALL, (uint8_t **)args);
            }
            break;
        case IOCTL_MS523_PCD_ANTICOLL:
            if(pdesc && args) {
                retval = ms523_pcd_anticoll(pdesc, (uint8_t **)args);
            }
            break;
        case IOCTL_MS523_PCD_SELECT:
            if(pdesc) {
                retval = ms523_pcd_select(pdesc);
            }
            break;
        default:
            break;
    }

    return retval;
}
