# Jose Jimenez, 2013,2014

CROSS_COMPILE ?= lm32-elf-
export CROSS_COMPILE

CC =		$(CROSS_COMPILE)gcc
LD =		$(CROSS_COMPILE)ld
OBJDUMP =	$(CROSS_COMPILE)objdump
OBJCOPY =	$(CROSS_COMPILE)objcopy
SIZE =		$(CROSS_COMPILE)size

WRPC_DIR = wrpc-sw

-include $(CURDIR)/.config

AUTOCONF = $(CURDIR)/include/generated/autoconf.h

# we miss CONFIG_ARCH_LM32 as we have no other archs by now
obj-y = arch/lm32/crt0.o arch/lm32/irq.o 
LDS-y = arch/lm32/ram.ld sbrkr.o

#fine delay including main file located on .mk
obj-$(CONFIG_DEBUGGER) += dbg_main.o

# our linker script is preprocessed, so have a rule here
%.ld: %.ld.S $(AUTOCONF) .config
	$(CC) -include $(AUTOCONF) -E -P $*.ld.S -o $@


cflags-y =	-ffreestanding -I- -include $(AUTOCONF) -Iinclude/std -Iinclude \
			-I.
cflags-y +=	-I$(CURDIR)/$(WRPC_DIR)/pp_printf

CFLAGS_PLATFORM  = -mmultiply-enabled -mbarrel-shift-enabled
LDFLAGS_PLATFORM = -mmultiply-enabled -mbarrel-shift-enabled \
	-nostdlib -T $(LDS-y)

include linux/linux.mk
include fmc-delay/fine-delay.mk
include shell/shell.mk
include lib/lib.mk
include printf.mk
include dev/dev.mk

obj-y += check-error.o

obj-y += sdb-lib/libsdbfs.a
cflags-y += -Isdb-lib

CFLAGS = $(CFLAGS_PLATFORM) $(cflags-y) -Wall \
	-ffunction-sections -fdata-sections -Os \
	-ggdb

LDFLAGS = $(LDFLAGS_PLATFORM) \
	-Wl,--gc-sections -Os -lgcc -lc

OBJS = $(obj-y)

#fine delay output included on .mk
output-$(CONFIG_DEBUGGER) = dbg

OUTPUT = $(output-y)

#REVISION=$(shell git describe --dirty --always)

all: wrpc dbg_core

dbg_core: remote_tools tools $(OUTPUT).ram $(OUTPUT).vhd $(OUTPUT).mif

.PRECIOUS: %.elf %.bin
.PHONY: all tools remote_tools clean

sdb-lib/libsdbfs.a:
	$(MAKE) -C sdb-lib

$(OUTPUT).elf: $(LDS-y) $(AUTOCONF) $(OUTPUT).o config.o
	${CC} -o $@  config.o $(OUTPUT).o $(LDFLAGS)
	${OBJDUMP} -d $(OUTPUT).elf > $(OUTPUT)_disasm.S
	$(SIZE) $@
	

$(OUTPUT).o: $(OBJS)
	$(LD) --gc-sections -e _start -r $(OBJS) -T bigobj.lds -o $@

config.o: .config
	sed '1,3d' .config > .config.bin
	dd bs=1 count=1 if=/dev/zero 2> /dev/null >> .config.bin
	$(OBJCOPY) -I binary -O elf32-lm32 -B lm32 \
		--rename-section .data=.data.config  .config.bin $@
	rm -f .config.bin

%.bin: %.elf
	${OBJCOPY} -O binary $^ $@

%.ram: tools %.bin
	./$(WRPC_DIR)/tools/genraminit $*.bin 0 > $@

%.vhd: tools %.bin
	./$(WRPC_DIR)/tools/genramvhd -s `. ./.config; echo $$CONFIG_RAMSIZE` $*.bin > $@

%.mif: tools %.bin
	./$(WRPC_DIR)/tools/genrammif $*.bin `. ./.config; echo $$CONFIG_RAMSIZE` > $@

$(AUTOCONF): silentoldconfig

clean:
	rm -f $(OBJS) *.o *.mif *.vhd *.elf *.bin *.ram *.S $(LDS)
	$(MAKE) -C sdb-lib clean
	$(MAKE) -C remote_tools clean
	$(MAKE) -C $(WRPC_DIR) clean

%.o:		%.c
	${CC} $(CFLAGS) $(INCLUDE_DIR) $(LIB_DIR) -c $*.c -o $@

tools:
	$(MAKE) -C $(WRPC_DIR)/tools -f Makefile

remote_tools:
	$(MAKE) -C remote_tools
	
wrpc: gitmodules
	$(MAKE) -C $(WRPC_DIR) etherbone_defconfig
	$(MAKE) -C $(WRPC_DIR)

# if needed, check out the submodules (first time only), so users
# who didn't read carefully the manual won't get confused
gitmodules:
	@test -d $(WRPC_DIR)/inlcude || echo "Checking out submodules"
	@test -d $(WRPC_DIR)/inlcude || git submodule update --init


# following targets from Makefile.kconfig
silentoldconfig:
	@mkdir -p include/config
	$(MAKE) -f Makefile.kconfig $@

scripts_basic config:
	$(MAKE) -f Makefile.kconfig $@

%config:
	$(MAKE) -f Makefile.kconfig $@

defconfig:
	$(MAKE) -f Makefile.kconfig dbg_defconfig

.config: silentoldconfig

# This forces more compilations than needed, but it's useful
# (we depend on .config and not on include/generated/autoconf.h
# because the latter is touched by silentoldconfig at each build)
$(obj-y): .config $(wildcard include/*.h)
