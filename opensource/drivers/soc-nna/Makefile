CROSS_COMPILE ?= mips-linux-gnu-

ISVP_ENV_KERNEL_DIR ?= $(PWD)/../../kernel-4.4.94
KDIR := ${ISVP_ENV_KERNEL_DIR}

MODULE_NAME := soc-nna

all: modules

.PHONY: modules clean

EXTRA_CFLAGS +=

$(MODULE_NAME)-objs := soc_nna_main.o platform.o

obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@find . -name '*.o' -o -name '*~' -o -name '.depend' -o -name '.*.cmd' \
		-o -name '*.mod.c' -o -name '.tmp_versions' -o -name '*.ko' \
		-o -name '*.symvers' -o -name 'modules.order' | xargs rm -rf
