obj-y += \
	shell/shell.o \
	shell/cmd_sdb.o \
	shell/cmd_help.o \
	shell/cmd_sleep.o

obj-$(CONFIG_MEM_CHECK_CMD) += shell/cmd_dbgmem.o
