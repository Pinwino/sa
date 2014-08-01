/* This work is part of the White Rabbit project
 * 
 * Jose Jimenez  <jjimenez.wr@gmail.com>, Copyright (C) 2014.
 * Released according to the GNU GPL version 3 (GPLv3) or later.
 * 
 * Code to operate wb_irq_timer.vhd This is a preliminar version. 
 * This is a preliminar version. The code is not finish.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <hw/irq_timer.h>
#include <errno.h>
#include <pp-printf.h>
#include <dbg.h>
#define mprintf pp_printf
#define vprintf pp_vprintf
#define sprintf pp_sprintf

uint8_t irq_timer_check_armed (struct irq_timer *itmr)
{
	return  ((irq_timer_readl(itmr, ARM_STAT) >> itmr->timer_id_num) & 0x1);
}

int irq_timer_arm (struct irq_timer *itmr, uint8_t option)
{
	switch (option)
	{
		case timer_arm:
			irq_timer_writel(itmr, irq_timer_readl(itmr, ARM_STAT) 
			| (set_1 << itmr->timer_id_num), ARM_SET);
		break;
		case timer_disarm:
			irq_timer_writel(itmr,  irq_timer_readl(itmr, ARM_STAT) 
			| (set_1 << itmr->timer_id_num), ARM_CLR);
		break;
		default:
			return -EINVAL;
		break;
	}
	return 0;
}

int irq_timer_time_mode (struct irq_timer  *itmr, uint8_t option)
{
	switch(option)
	{
		case abs_time:
			irq_timer_writel(itmr, irq_timer_readl(itmr, SRC_STAT)
			| (set_1 << itmr->timer_id_num), SRC_CLR);
		break;
		case diff_time_periodic:
			irq_timer_writel(itmr, irq_timer_readl(itmr, SRC_STAT)
			| (set_1 << itmr->timer_id_num), SRC_SET);
			irq_timer_writel(itmr, irq_timer_readl(itmr, D_MODE_STAT)
			| (set_1 << itmr->timer_id_num), D_MODE_SET);
		break;
		case diff_time_1time:
			irq_timer_writel(itmr, irq_timer_readl(itmr, SRC_STAT)
			| (set_1 << itmr->timer_id_num), SRC_SET);
			irq_timer_writel(itmr, irq_timer_readl(itmr, D_MODE_STAT)
			| (set_1 << itmr->timer_id_num), D_MODE_CLR);
		break;
		default:
			return -EINVAL;
		break;
	}
	return 0;
}

int irq_timer_sel_cascade(struct irq_timer  *itmr, uint8_t option)
{
	switch(option)
	{
		case cascade_enable:
			irq_timer_writel(itmr, irq_timer_readl(itmr, CSC_STAT)
			| (set_1 << itmr->timer_id_num), SRC_SET);
		break;
		case cascade_disable:
			irq_timer_writel(itmr, irq_timer_readl(itmr, CSC_STAT)
			| (set_1 << itmr->timer_id_num), SRC_CLR);
		break;
		default:
			return -EINVAL;
		break;
	}
	return 0;
}

void irq_timer_set_time(struct irq_timer  *itmr, unsigned long  long expires)
{
	irq_timer_writel(itmr, (uint32_t) (expires >> 32), TM_TIME_HI);
	irq_timer_writel(itmr, (uint32_t) expires, TM_TIME_LO);
}


/*int prog_irq_timer (struct irq_timer *itmr){

	//if (irq_timer_check_armed){}
	//return 0;		
	
}*/
