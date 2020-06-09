# Top Makefile
# 
# @file /Makefile
# 
# Copyright (C) 2019
#
# all       --> build all
# clean     --> clean project
# distclean --> clean project, rm obj dir
# tshow     --> show optimize settings
# Example:
#   make Optimize=0 all tshow CROSS_COMPILER=arm-none-eabi-
#
# @author HinsShum hinsshum@qq.com

include Makefile.common

ifndef PROGRAM
PROGRAM := main
endif

CFLAGS += -DEFM32LG280F256 -DEFM32_HFXO_FREQ="16000000" -DCONFIG_PRINTK

all: $(DIRS_MAKEFILE) $(PROGRAM)

.PHONY: $(DIRS_MAKEFILE) clean distclean tshow

$(DIRS_MAKEFILE):
	$(MAKE) -C $@

$(PROGRAM):
	$(CC) -o $@.elf $(LDFLAGS) $(wildcard $(DIRS_OBJ)/*.o)
	$(OBJCOPY) -O ihex $@.elf $@.hex
	$(OBJCOPY) -O binary $@.elf $@.bin
	$(OBJDUMP) -D -t -h $@.elf > $@.dis

clean:
	rm -f $(PROGRAM).elf $(PROGRAM).hex $(PROGRAM).bin $(PROGRAM).dis

distclean: clean
	rm $(DIRS_OBJ) -r

tshow:
	@echo "######################################################################################################"
	@echo "################# optimize settings: $(INFOTEXTSRC)"
	@echo "######################################################################################################"
