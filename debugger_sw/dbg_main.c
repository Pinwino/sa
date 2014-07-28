/*
 * This work is part of the White Rabbit project
 *
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdio.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>

#include <w1.h>
#include "uart.h"
#include "eeprom.h"
#include <pp-printf.h>
#include "shell.h"
#include "irq.h"
#include "linux/jiffies.h"
#include "linux/delay.h"

#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

void _irq_entry(void) {}


extern uint32_t _endram;
extern uint32_t _fstack;
#define ENDRAM_MAGIC 0xbadc0ffe

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

/*static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) {
		mprintf("Stack overflow!\n");
		msleep(1000);
	}
}*/


int main(void)
{
	
	sdb_find_devices();
	//uart_init_sw();
	uart_init_hw();
	
	mprintf("Running....\n");

	shell_exec("dbgmem");
	shell_exec("dbgmem 0x80000");
	shell_exec("dbgmem -b 0x80000 0x1");
	shell_exec("dbgmem -b0x80000 0x1");
	shell_exec("dbgmem -b0x0000");
	
	for (;;){
		shell_interactive();
		//check_stack();
	}
}
