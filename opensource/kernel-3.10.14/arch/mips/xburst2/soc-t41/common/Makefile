

## WARNING! DO NOT change this lists order because power sleep order depends on it!! ##

obj-y += setup.o
obj-y += reset.o
obj-y += serial.o

obj-y += clk/

obj-y += gpio.o
# only for debug
#obj-y += pmmon.o
#obj-y += irq.o
obj-y += cpm_usb.o

#obj-y += cpu_spinlock_test.o
#obj-y += cpu_ccu_test.o
#obj-y += multithread_test.o

obj-y += platform.o

#obj-y += pm.o

#obj-y += ahbm.o
def_dpu = $(filter-out undefined,$(foreach v,$(1),$(origin $(v))))
ifneq ($(call def_dpu,CONFIG_FB_JZ_V1 CONFIG_FB_JZ_V16),)
	obj-y += libdmmu_dpu.o
else
	obj-y += libdmmu.o
endif

obj-y += ddr.o
obj-y += proc.o
obj-y += jzfblcd/
