CROSS_COMPILE ?= mips-linux-gnu-

KDIR := $(ISVP_ENV_KERNEL_DIR)

MODULE_NAME := hash_dev

all: modules

.PHONY: modules clean

$(MODULE_NAME)-objs := jz-hash.o
obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
