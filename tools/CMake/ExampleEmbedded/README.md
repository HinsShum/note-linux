# efm32lg_gcc_demo
use the general makefile to manage the project and use the cross compilation tool chain as compiler



## Makefile 层次结构

本demo的makefile有两个层次，分为顶层Makefile和子层Makefile。

- 顶层Makefile由主目录下的`Makefile`和`Makefile.common`构成

- 子层Makefile由各个目录下的`Makefile`构成

顶层Makefile在执行编译过程中会循环进入子层Makefile所在目录并编译该目录下的待编译文件，因此在构建项目的过程中无需改动顶层Makefile的两个文件，只需针对子层Makefile文件所在目录下的具体编译情况进行修改部分变量值即可。所有子层`Makefile`文件内容均相同。

比如`/bsp/efm32lgxx/`目录下的`Makefile`如下：

```makefile
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
```

---

**下面针对三种情况介绍如何改动此`Makefile`文件**

1. 新增待编译文件`bsp_led.c`  --> 无需改动`Makefile`文件
2. 删除文件`bsp_led.c`             --> 无需改动`Makefile`文件
3. 保留`bsp_led.c`但不编译     --> 修改变量`OBJS_FILTEROUT := bsp_led.o`，编译时会剔除该`bsp_led.c`文件



## 注意事项

1. 主目录下不允许放置待编译文件
2. 所有子目录下有待编译文件的目录中必须放置子层`Makefile`文件
3. 所有头文件必须集中放置，且放置目录必须为`inc`或者`include`，这样做的好处在于避免手动添加头文件路径，由顶层`Makefile`根据`inc`或者`include`文件夹名自动识别

