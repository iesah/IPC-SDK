#
# linux/arch/mips/boot/compressed/Makefile
#
# create a compressed zImage from the original vmlinux
#

targets		:= zImage uImage vmlinuz vmlinux.bin.gz head.o misc.o piggy.o dummy.o

OBJS 		:= $(obj)/head.o $(obj)/misc.o

LD_ARGS 	:= -m elf32ltsmip -T $(obj)/ld.script -Ttext 0x80F00000 -Bstatic -EL
OBJCOPY_ARGS 	:= -O elf32-tradlittlemips

ENTRY 		:= $(obj)/entry
FILESIZE 	:= $(obj)/filesize

KBUILD_CFLAGS := $(shell echo $(KBUILD_CFLAGS) | sed -e "s/-pg//")
KBUILD_CFLAGS := $(filter-out -fstack-protector, $(KBUILD_CFLAGS))
KBUILD_CFLAGS := $(LINUXINCLUDE) $(KBUILD_CFLAGS) -D__KERNEL__ \
       -DBOOT_HEAP_SIZE=$(BOOT_HEAP_SIZE) -D"VMLINUX_LOAD_ADDRESS_ULL=$(VMLINUX_LOAD_ADDRESS)ull"


drop-sections	= .reginfo .mdebug .comment .note .pdr .options .MIPS.options
strip-flags	= $(addprefix --remove-section=,$(drop-sections))

LOADADDR=0x80010000

$(obj)/vmlinux.bin.gz: vmlinux
	rm -f $(obj)/vmlinux.bin.gz
	$(OBJCOPY) -O binary $(strip-flags) vmlinux $(obj)/vmlinux.bin
	gzip -v9f $(obj)/vmlinux.bin

$(obj)/head.o: $(obj)/head.S $(obj)/vmlinux.bin.gz vmlinux
	echo $(CC)
	$(CC) $(KBUILD_AFLAGS) $(a_flags) \
	-DIMAGESIZE=$(shell sh $(FILESIZE) $(obj)/vmlinux.bin.gz) \
	-DKERNEL_ENTRY=$(shell sh $(ENTRY) $(NM) vmlinux ) \
	-DLOADADDR=0x80010000 \
	-I./include \
	-c -o $(obj)/head.o $<

$(obj)/vmlinuz: $(OBJS) $(obj)/ld.script $(obj)/vmlinux.bin.gz $(obj)/dummy.o
	$(OBJCOPY) \
		--add-section=.image=$(obj)/vmlinux.bin.gz \
		--set-section-flags=.image=contents,alloc,load,readonly,data \
		$(obj)/dummy.o $(obj)/piggy.o
	$(LD) $(LD_ARGS) -o $@ $(OBJS) $(obj)/piggy.o
	$(OBJCOPY) $(OBJCOPY_ARGS) $@ $@ -R .comment -R .stab -R .stabstr -R .initrd -R .sysmap

zImage: $(obj)/vmlinuz
	$(OBJCOPY) -O binary $(obj)/vmlinuz $(obj)/zImage
	@rm -f $(obj)/vmlinuz

vmlinux.bin: $(VMLINUX)
	$(OBJCOPY) -O binary $(strip-flags) $(VMLINUX) $(obj)/vmlinux.bin

uImage: $(VMLINUX) vmlinux.bin
	rm -f $(obj)/vmlinux.bin.gz
	gzip -9 $(obj)/vmlinux.bin
	mkimage -A mips -O linux -T kernel -C gzip \
		-a $(LOADADDR) -e $(shell sh ./$(obj)/tools/entry $(NM) $(VMLINUX) ) \
		-n 'Linux-$(KERNELRELEASE)' \
		-d $(obj)/vmlinux.bin.gz $(obj)/uImage
xImage: zImage
	mkimage -A mips -O linux -T kernel -C none \
		-a 0x80F00000 -e 0x80F00000 \
		-n 'Linux-$(KERNELRELEASE)' \
		-d $(obj)/zImage $(obj)/xImage
	@rm -f $(obj)/zImage
