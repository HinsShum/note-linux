CC = gcc
CFLAGS = -ggdb -Wall -Werror -Wformat=0

#编译临时文件目录
OBJDIR := Obj

#windows和bash中都有find.exe，但是语法不一样，需安装bash后再执行该Makefile
FIND := $(strip $(findstring /usr/bin/find,$(shell which find -a)))
ifndef FIND
    $(error "error: Please install the Unix environment first!")
endif

#获取所有目录
all_dirs := $(strip $(patsubst ./%,%,$(shell $(FIND) . -type d)))

#移除非源码目录
filterout_dirs := .git .vscode .slickedit Obj build
src_dirs := $(filter-out $(foreach dd,$(filterout_dirs),$(foreach d,$(all_dirs),$(if $(findstring $(dd),$(d)),$(d)))),$(all_dirs))

#头文件目录
INCLUDE := $(patsubst %,-I%,$(src_dirs))

#设置编译查找目录
vpath %.c $(src_dirs)

#创建编译临时目录
D := $(findstring $(OBJDIR),$(shell ls))
ifndef D
    $(shell mkdir $(OBJDIR))
endif

#在所有目录中搜寻.c文件，搜寻到的.c文件只取文件名不要路径
srcs_all := $(foreach d,$(src_dirs),$(notdir $(wildcard $(d)/*.c)))

#不需要编译的文件(以_开头的.c文件)
srcs_out := $(foreach d,$(src_dirs),$(notdir $(wildcard $(d)/_*.c)))

#除去不需要的文件
srcs := $(filter-out $(srcs_out),$(srcs_all))

#将所有.c文件编译后的,o文件存放在一个目录Obj下
objs := $(srcs:%.c=$(OBJDIR)/%.o)

main.exe: $(objs)
	$(CC) $^ -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -c $< $(INCLUDE) $(CFLAGS) -o $@

.PHONY: clean test
clean:
	rm $(OBJDIR) -rf
	rm *.exe -f

test:
	@echo $(src_dirs)
#	@echo $(dirss)
#	@echo $(objs)
