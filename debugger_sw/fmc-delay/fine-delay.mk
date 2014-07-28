#kernel files
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
         calibration.o #patch
	     
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

#main
FINE_DEL_DIR = fmc-delay
obj-$(CONFIG_FINE_DEL_NODE) += $(FINE_DEL_DIR)/fd_stand_alone_main.o

#ouput file
output-$(CONFIG_FINE_DEL_NODE) = fd_std
