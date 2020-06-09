# @file /lib/siliconlabs/efm32lgxx/emlib/src/Makeifle
#
# Copyright (C) 2019
#
# @author HinsShum hinsshum@qq.com

OBJS_ALL := $(patsubst %.c,$(TOP)/$(DIRS_OBJ)/%.o,$(notdir $(wildcard *.c)))
OBJS_ALL += $(patsubst %.S,$(TOP)/$(DIRS_OBJ)/%.o,$(notdir $(wildcard *.S)))

OBJS_FILTEROUT := em_acmp.o em_aes.o em_assert.o em_burtc.o em_can.o em_cryotimer.o em_crypto.o em_csen.o em_dac.o em_dbg.o
OBJS_FILTEROUT += em_dma.o em_ebi.o em_gpcrc.o em_i2c.o em_idac.o em_int.o em_lcd.o em_ldma.o em_lesense.o em_mpu.o em_opamp.o
OBJS_FILTEROUT += em_pcnt.o em_prs.o em_qspi.o em_rtc.o em_rtcc.o em_timer.o em_vcmp.o em_vdac.o em_wdog.o
OBJS := $(filter-out $(foreach dd,$(OBJS_FILTEROUT),$(foreach d,$(OBJS_ALL),$(if $(findstring $(dd),$(d)),$(d)))),$(OBJS_ALL))

DIR_PATH := $(shell pwd)

all: $(OBJS)
	@echo "Building $(DIR_PATH) done."

$(TOP)/$(DIRS_OBJ)/%.o: %.S
	$(CC) $(CFLAGS) -o $@ $< -c

$(TOP)/$(DIRS_OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $< -c

.PHONY: test

test:
	@echo "It is $(DIR_PATH)/Makefile"