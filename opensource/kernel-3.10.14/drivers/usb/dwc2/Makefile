ccflags-$(CONFIG_USB_DWC2_DEBUG)	:= -DDEBUG
ccflags-$(CONFIG_USB_DWC2_VERBOSE)	+= -DVERBOSE_DEBUG

obj-$(CONFIG_USB_JZ_DWC2)			+= dwc2.o

dwc2-y	:= core.o

ifneq ($(CONFIG_USB_DWC2_DEVICE_ONLY),y)
dwc2-y	+= host-ddma.o rh.o
endif

ifneq ($(CONFIG_USB_DWC2_HOST_ONLY),y)
dwc2-y	+= gadget.o ep0.o
endif

ifneq ($(CONFIG_DEBUG_FS),)
	dwc2-y				+= debugfs.o
endif

##
# Platform-specific glue layers go here
#
# NOTICE: Make sure your glue layer doesn't depend on anything
# which is arch-specific and that it compiles on all situations.
#
# We want to keep this requirement in order to be able to compile
# the entire driver (with all its glue layers) on several architectures
# and make sure it compiles fine. This will also help with allmodconfig
# and allyesconfig builds.
#
# The only exception is the PCI glue layer, but that's only because
# PCI doesn't provide nops if CONFIG_PCI isn't enabled.
##

dwc2-y		+= dwc2_jz.o
