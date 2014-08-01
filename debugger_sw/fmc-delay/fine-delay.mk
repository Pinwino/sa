# This work is part of the White Rabbit project
#
# Jose Jimenez  <jjimenez.wr@gmail.com>, Copyright (C) 2014.
# Released according to the GNU GPL version 3 (GPLv3) or later. 
#

#kernel
FINE_DEL_KERNEL_DIR = fmc-delay/fine-delay-sw/kernel
obj-$(CONFIG_FINE_DEL_NODE) += \
	$(FINE_DEL_KERNEL_DIR)/spi.o \
	$(FINE_DEL_KERNEL_DIR)/gpio.o \
	$(FINE_DEL_KERNEL_DIR)/pll.o \
	$(FINE_DEL_KERNEL_DIR)/onewire.o \
	$(FINE_DEL_KERNEL_DIR)/time.o \
	$(FINE_DEL_KERNEL_DIR)/i2c.o \
	$(FINE_DEL_KERNEL_DIR)/calibrate.o \
	$(FINE_DEL_KERNEL_DIR)/acam.o \
	$(FINE_DEL_KERNEL_DIR)/calibration.o
#/kernel

#commands
FINE_DEL_CMP_DIR = fmc-delay/fine-delay-cmds
obj-$(CONFIG_FINE_DEL_NODE) += \
	$(FINE_DEL_CMP_DIR)/cmd_fmc_fdelay_pulse.o \
	$(FINE_DEL_CMP_DIR)/cmd_fmc_fdelay_term.o \
	$(FINE_DEL_CMP_DIR)/cmd_fmc_fdelay_status.o \
	$(FINE_DEL_CMP_DIR)/cmd_fmc_fdelay_time.o \
	$(FINE_DEL_CMP_DIR)/cmd_fmc_fdelay_temp.o \
	$(FINE_DEL_CMP_DIR)/tools-util.o \
	$(FINE_DEL_CMP_DIR)/fdelay-output.o
#/commands

#main
FINE_DEL_DIR = fmc-delay
obj-$(CONFIG_FINE_DEL_NODE) += $(FINE_DEL_DIR)/fd_stand_alone_main.o
#/main

#ouput
output-$(CONFIG_FINE_DEL_NODE) = fd_std
#/ouput
