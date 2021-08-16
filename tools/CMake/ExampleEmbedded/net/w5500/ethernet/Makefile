# @file /net/w5500/ethernet/Makefile
#
# Copyright (C) 2019
#
# @author HinsShum hinsshum@qq.com

OBJS_ALL := $(patsubst %.c,$(TOP)/$(DIRS_OBJ)/%.o,$(notdir $(wildcard *.c)))
OBJS_ALL += $(patsubst %.S,$(TOP)/$(DIRS_OBJ)/%.o,$(notdir $(wildcard *.S)))

OBJS_FILTEROUT :=
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
