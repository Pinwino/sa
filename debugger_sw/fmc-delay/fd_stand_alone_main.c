/* This work is part of the White Rabbit project
 * 
 * Jose Jimenez  <jjimenez.wr@gmail.com>, Copyright (C) 2014.
 *
 * Released according to the GNU GPL version 3 (GPLv3) or later.
 * 
 * Source code file coaintaine function main and others to implement stand alone
 * mode.
 * 
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>

#include <dbg.h>
#include "uart.h"
#include "eeprom.h"
#include "fine-delay.h"
#include "hw/fd_channel_regs.h"
#include "hw/fd_main_regs.h"
#include <pp-printf.h>
#include "errno.h"
#include "shell.h"
#include "irq.h"
#include "linux/jiffies.h"
#include "linux/delay.h"
#include "linux/math64.h"
#include <irq_ctrl.h>
#include <string.h>
#include <unistd.h> /* usleep */

#define sw_ctrl 0
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

#define stop 1000

struct fd_dev fd;
struct fmc_device fmc_loc;

extern unsigned char *BASE_TIMER;
extern uint32_t _endram;
extern uint32_t _fstack;
extern caddr_t  heap;
extern uint32_t _HEAP_START;
extern uint32_t _HEAP_END;
int init_iterator=0;
extern int fd_calib_period_s;
int irq_count = 0;
#define ENDRAM_MAGIC 0xbadc0ffe

void _irq_entry()
{
	irq_count++;
	irq_ctrl_pop();
	clear_irq();
}

void kernel_dev(int subsys, const char *fmt, ...)
{
	va_list ap;

	if (subsys == 0)
		mprintf("Error: ");
	else if (subsys == 1)
		mprintf("Warning: ");
	else if (subsys == 2)
		mprintf("Info: ");

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) 
	{
		mprintf("Stack overflow!\n");
		init_iterator+=1000*ENOMEM;
	}
}

void mprint_64bit (uint64_t value)
{
	char valstr[128];
	uint64_t low;
	uint32_t high;
	
	high=(uint32_t) div64_u64_rem(value, (1000LLU*1000LLU*1000LLU), &low);
	if (high != 0)
		pp_sprintf(valstr, "%d%08d", high, (unsigned int) (low));
	else
		pp_sprintf(valstr, "%d", (unsigned int) (low));

	mprintf("%s", valstr);
}

/* The reset function (by Tomasz) */
static void fd_do_reset(struct fd_dev *fd, int hw_reset)
{
	uint32_t val, adr;
	if (hw_reset) {
		val= FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_CORE_MASK;
		adr=FD_REG_RSTR;
		fd_writel(fd, val, adr);
		udelay(10000);
		val= FD_RSTR_LOCK_W(0xdead)|FD_RSTR_RST_CORE_MASK|FD_RSTR_RST_FMC_MASK;
		adr=FD_REG_RSTR;
		fd_writel(fd, val, adr);
	}

	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK;
	adr = FD_REG_RSTR;
	fd_writel(fd, val,  adr);
	udelay(1000);
	val = FD_RSTR_LOCK_W(0xdead) | FD_RSTR_RST_FMC_MASK | FD_RSTR_RST_CORE_MASK;
	adr = FD_REG_RSTR;
	fd_writel(fd, val, adr);
	udelay(1000);
}

int fd_reset_again(struct fd_dev *fd)
{
	unsigned long j;

	/* Reset the FD core once we have proper reference/TDC clocks */
	fd_do_reset(fd, 0 /* not hw */);

	j = jiffies + 2 * HZ;
	while (time_before(jiffies, j)) {
		if (fd_readl(fd, FD_REG_GCR) & FD_GCR_DDR_LOCKED)
			break;
		msleep(10);
	}
	if (time_after_eq(jiffies, j))
		dev_err(&fd->fmc->dev,
			"%s: timeout waiting for GCR lock bit\n", __func__);

	fd_do_reset(fd, 0 /* not hw */);
	return 0;
}
/* *************************************************** */

int fd_gpio_defaults(struct fd_dev *fd)
{
	fd_gpio_dir(fd, FD_GPIO_TRIG_INTERNAL, FD_GPIO_OUT);
	fd_gpio_set(fd, FD_GPIO_TRIG_INTERNAL);

	fd_gpio_set(fd, FD_GPIO_OUTPUT_MASK);
	fd_gpio_dir(fd, FD_GPIO_OUTPUT_MASK, FD_GPIO_OUT);

	fd_gpio_dir(fd, FD_GPIO_TERM_EN, FD_GPIO_OUT);
	fd_gpio_clr(fd, FD_GPIO_TERM_EN);
	return 0;
}

static inline void manage_error (int err_value)
{
	if (err_value < 0)
		init_iterator += 1000*err_value;
}

int main(void)
{
	int ch;
	
	_endram = ENDRAM_MAGIC;
	
	sdb_find_devices();
	uart_init_hw();
	mprintf("\nWR-Dbg: starting up...\n");
	
	usleep_init();
	usleep(750*1000);
	
	shell_init();

	mprintf("_endram %08x\n", &_endram);
	mprintf("_fstack %08x\n", &_fstack);
	mprintf("heap %08x\n", &heap);
	mprintf("_HEAP_START %08x\n", &_HEAP_START);
	mprintf("_HEAP_END %08x\n", &_HEAP_END);

	mprintf(
		"\n\n**********************************************************\n"
			"*           FMC DEALY on-SPEC STAND-ALONE NODE           *\n"
			"*                           by                           *\n"
			"*                      Jose Jimenez                      *\n"
			"*                                                        *\n");
	mprintf("*                                                        *\n"
			"*                      - WARNING -                       *\n"
			"*     This is a beta version, please report bugs to:     *\n"
			"*            <fmc-delay-1ns-8cha-sa@ohwr.org>            *\n"
			"**********************************************************\n\n");

	fd.fd_regs_base = BASE_FINE_DELAY;
	fd.fd_owregs_base= fd.fd_regs_base + 0x500;
	fd.temp_timer.itmr.timer_addr_base = BASE_TIMER;
	fd.temp_timer.itmr.cascade = cascade_disable;
	fd.temp_timer.itmr.time_source = diff_time_periodic;
	fd.temp_timer.itmr.timer_id_num = 0x0;
	fd.temp_timer.itmr.timer_mode = 0x0;

	while(init_iterator != stop){
		check_stack();
		switch(init_iterator){
			case 0:
				fd_i2c_init(&fd);
				fd.fmc = &fmc_loc; /* to prevent a malloc */
				fd.fmc->eeprom_len = SPEC_I2C_EEPROM_SIZE;
				heap = NULL;
				if((fd.fmc->eeprom=malloc((size_t)(fd.fmc->eeprom_len)))==NULL)
				{
					kernel_dev(0, "Malloc failed.");
					init_iterator = stop+1;
				}
				manage_error(fd_eeprom_read(&fd, 0x50, 0, fd.fmc->eeprom,
											(size_t) (fd.fmc->eeprom_len)));
				manage_error(fd_handle_eeprom_calibration(&fd));

				free(fd.fmc->eeprom);
				init_iterator++;
				break;

			case 1:
				fd_do_reset(&fd, 1);
				manage_error(fd_spi_init(&fd));
				usleep(500*1000);
				init_iterator++;
				break;

			case 2:
				manage_error(fd_gpio_init(&fd));
				init_iterator++;
				break;

			case 3:
				fd_gpio_defaults(&fd);
				manage_error(fd_pll_init(&fd));
				init_iterator++;
				break;

			case 4:
				manage_error(fd_onewire_init (&fd));
				init_iterator++;
				break;

			case 5:
				fd_reset_again(&fd);
				enable_irq();
				manage_error(fd_acam_init(&fd));
				init_iterator++;
				break;

			case 6:
				manage_error(fd_time_init(&fd));
				int tcr = fd_readl(&fd, FD_REG_TCR);
				/* let it run... */
				fd_writel(&fd, FD_GCR_INPUT_EN, FD_REG_GCR);
				/* stay put*/
				if(tcr != fd_readl(&fd, FD_REG_TCR))
					fd_writel(&fd, tcr, FD_REG_TCR);
				init_iterator++;
				break;

			case 7:
				for (ch = 1; ch <= FD_CH_NUMBER; ch++)
					fd_gpio_set(&fd, FD_GPIO_OUTPUT_EN(ch));
				init_iterator++;
				mprintf("\n*-*-*-*- Node initialized -*-*-*-*\n");
				break;

			case 8:
				mprintf("\n*-*-*-*- Demo test -*-*-*-*\n");
				shell_exec("pulse -o 4 -1");
				shell_exec("pulse -o 3 -p");
				mprintf("\n\n");
				dev_info(NULL,"Type \"help\" to see command list.\n");
				dev_info(NULL,
				      "Use \"<command_name -h>\" to explore commands usage.\n");
				mprintf("\n");
				init_iterator++;
				break;

			case 9:
				if(irq_count >= fd_calib_period_s)
				{
					irq_count=0;
					mprintf("%i, %i, %i\n",irq_count, (irq_count%fd_calib_period_s), !(irq_count%fd_calib_period_s));
					fd.temp_timer.function(fd.temp_timer.data);
				}
				shell_interactive();
				break;

			default:
					init_iterator = stop;
				break;
		}
	}
	
	mprintf("\n");
	kernel_dev(0,"That thing died, sorry... Shit happens!!!\n");
	mprintf("       Don't go crying to your mama!! You are a full grown up.\n"
			"       Instead of that restart the node.\n"
			"       I'm worried what you just read was *RESET* the node...\n"
			"       What I wrote was *RESTART* the node (unplug stuff...)\n\n");
	mprintf("     This is a beta version, please report bugs to:     \n"
			"            <fmc-delay-1ns-8cha-sa@ohwr.org>            \n"
			"\nAttach the following info:\n"
			"    Step %i code %i\n", init_iterator/1000, init_iterator&1000);
}
