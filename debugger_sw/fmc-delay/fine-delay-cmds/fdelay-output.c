/*
 * output-related functions
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <fdelay-lib.h>
#include <fine-delay.h>
#include "hw/fd_main_regs.h"
#include "hw/fd_channel_regs.h"

#define MAX_EXT_ATTR 32
#define NSEC_PER_SEC 1000*1000*1000

void fdelay_pico_to_time(uint64_t *pico, struct fdelay_time *time)
{
	uint64_t p = *pico;

	time->utc = div64_u64_rem(p, (1000ULL * 1000ULL * 1000ULL * 1000ULL), &p);
	time->coarse = (uint32_t) div64_u64_rem(p, 8000ULL, &p);
	time->frac = (uint32_t) div64_u64_rem((p * 4096), 8000ULL, &p);
}

void fdelay_time_to_pico(struct fdelay_time *time, uint64_t *pico)
{
	uint64_t p;

	p = time->frac * 8000 / 4096;
	p += (uint64_t) time->coarse * 8000LL;
	p += time->utc * (1000ULL * 1000ULL * 1000ULL * 1000ULL);
	*pico = p;
}

static void fdelay_add_ps(struct fdelay_time *p, uint64_t ps)
{
	uint64_t tmp;
	uint32_t coarse, frac;

	/* FIXME: this silently fails with ps > 10^12 = 1s */
	//coarse = ps / 8000;
	tmp = div64_u64_rem(ps, 8000LLU, &ps);
	coarse = (uint32_t) tmp;
	//frac = ((ps % 8000) << 12) / 8000;
	frac = div_u64_rem((ps << 12), 8000LLU, NULL);

	p->frac += frac;
	if (p->frac >= 4096) {
		p->frac -= 4096;
		coarse++;
	}
	p->coarse += coarse;
	if (p->coarse >= 125*1000*1000) {
		p->coarse -= 125*1000*1000;
		p->utc++;
	}
}

static void fdelay_sub_ps(struct fdelay_time *p, uint64_t ps)
{
	uint64_t tmp;
	uint32_t coarse_neg, frac_neg;

	/* FIXME: this silently fails with ps > 10^12 = 1s */
	tmp = div64_u64_rem(ps, 8000LLU, &ps);
	coarse_neg = (uint32_t) tmp;
	frac_neg = div_u64_rem((ps << 12), 8000LLU, NULL);

	if (p->frac < frac_neg) {
		p->frac += 4096;
		coarse_neg++;
	}
	p->frac -= frac_neg;

	if (p->coarse < coarse_neg) {
		p->coarse += 125*1000*1000;
		p->utc--;
	}
	p->coarse -= coarse_neg;
}

static void fdelay_add_signed_ps(struct fdelay_time *p, signed ps)
{
	if (ps > 0)
		fdelay_add_ps(p, ps);
	else
		fdelay_sub_ps(p, -ps);
}

int fdelay_config_pulse(struct fd_dev *fd, int ch, struct fdelay_pulse *pulse)
{
	uint32_t a[MAX_EXT_ATTR];
	int i, dcr = 0, v=0, channel = ch;
	struct timespec delta, width, delay;
	int mode = pulse->mode & 0x7f;
	uint32_t input_offset, output_offset, output_user_offset;
	
	if (mode == FD_OUT_MODE_DELAY || mode == FD_OUT_MODE_DISABLED) {
		if(pulse->rep < 0 || pulse->rep > 16) /* delay mode allows trains of 1 to 16 pulses. */
			return -EINVAL;

		/* check delay lower limits. FIXME: raise an alarm */
		if (((uint32_t)(pulse->start.utc)) == 0 && pulse->start.coarse * 8  < 600) // 600 ns min delay
			return -EINVAL;

	}
	
	output_offset = fd->calib.zero_offset[channel];
	output_user_offset = fd->ch_user_offset [channel];
	input_offset = fd->calib.tdc_zero_offset;
	
	switch(mode)
	{
		case FD_OUT_MODE_DISABLED:
		/* hack for Steen/COHAL: if channel is disabled, apply delay-mode offsets */
		case FD_OUT_MODE_DELAY:
			fdelay_add_signed_ps(&pulse->start, (signed)output_offset);
			fdelay_add_signed_ps(&pulse->end, (signed)output_offset);
			fdelay_add_signed_ps(&pulse->start, (signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->end, (signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->start, (signed)input_offset);
			fdelay_add_signed_ps(&pulse->end, (signed)input_offset);
		break;
		case FD_OUT_MODE_PULSE:
			fdelay_add_signed_ps(&pulse->start, (signed)output_offset);
			fdelay_add_signed_ps(&pulse->end, (signed)output_offset);
			fdelay_add_signed_ps(&pulse->start, (signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->end, (signed)output_user_offset);
		break;
	}
	

	/*a[FD_ATTR_OUT_START_H] = (uint32_t) (pulse->start.utc >> 32);
	a[FD_ATTR_OUT_START_L] = (uint32_t) (pulse->start.utc);
	a[FD_ATTR_OUT_START_COARSE] = pulse->start.coarse;
	a[FD_ATTR_OUT_START_FINE] = pulse->start.frac;
	a[FD_ATTR_OUT_END_H] = (uint32_t)(pulse->end.utc >> 32);
	a[FD_ATTR_OUT_END_L] = (uint32_t) (pulse->end.utc);
	a[FD_ATTR_OUT_END_COARSE] = pulse->end.coarse;
	a[FD_ATTR_OUT_END_FINE] = pulse->end.frac;
	a[FD_ATTR_OUT_DELTA_L] = (uint32_t) (pulse->loop.utc); /* only 0..f *
	a[FD_ATTR_OUT_DELTA_COARSE] = pulse->loop.coarse; /* only 0..f *
	a[FD_ATTR_OUT_DELTA_FINE] = pulse->loop.frac; /* only 0..f *
	
	//fd_ch_writel(fd, ch, fd->ch[ch].frr_cur, 			FD_REG_FRR);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_START_H], 		FD_REG_U_STARTH);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_START_L], 		FD_REG_U_STARTL);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_START_COARSE], 	FD_REG_C_START);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_START_FINE], 	FD_REG_F_START);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_END_H], 			FD_REG_U_ENDH);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_END_L],      	FD_REG_U_ENDL);	
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_END_COARSE], 	FD_REG_C_END);	
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_END_FINE],   	FD_REG_F_END);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_DELTA_L],      	FD_REG_U_DELTA);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_DELTA_COARSE], 	FD_REG_C_DELTA);
	fd_ch_writel(fd, ch, a[FD_ATTR_OUT_DELTA_FINE],   	FD_REG_F_DELTA);*/
	
	fd_ch_writel(fd, ch, (uint32_t) (pulse->start.utc >> 32), FD_REG_U_STARTH);
	fd_ch_writel(fd, ch, ((uint32_t) (pulse->start.utc)),     FD_REG_U_STARTL);
	fd_ch_writel(fd, ch, pulse->start.coarse,                 FD_REG_C_START);
	fd_ch_writel(fd, ch, pulse->start.frac,                   FD_REG_F_START);
	fd_ch_writel(fd, ch, (uint32_t)(pulse->end.utc >> 32),    FD_REG_U_ENDH);
	fd_ch_writel(fd, ch, (uint32_t) (pulse->end.utc),         FD_REG_U_ENDL);	
	fd_ch_writel(fd, ch, pulse->end.coarse,                   FD_REG_C_END);	
	fd_ch_writel(fd, ch, pulse->end.frac,                     FD_REG_F_END);
	fd_ch_writel(fd, ch, (uint32_t) (pulse->loop.utc),        FD_REG_U_DELTA);
	fd_ch_writel(fd, ch, pulse->loop.coarse,                  FD_REG_C_DELTA);
	fd_ch_writel(fd, ch, pulse->loop.frac,                    FD_REG_F_DELTA);
	
	
	if (mode == FD_OUT_MODE_DELAY || mode == FD_OUT_MODE_DISABLED) {
		dcr = 0;
		fd_ch_writel(fd, ch, FD_RCR_REP_CNT_W(pulse->rep - 1)
			     | (pulse->rep < 0 ? FD_RCR_CONT : 0), FD_REG_RCR);
	} else {
		dcr = FD_DCR_MODE;
		fd_ch_writel(fd, ch, FD_RCR_REP_CNT_W(pulse->rep < 0 ? 0 : pulse->rep - 1)
			    | (pulse->rep < 0 ? FD_RCR_CONT : 0), FD_REG_RCR);
	}
	
	/*
	 * For narrowly spaced pulses, we don't have enough time to reload
	 * the tap number into the corresponding SY89295.
	 * Therefore, the width/spacing resolution is limited to 4 ns.
	 * We put the threshold at 200ns, i.e. when coarse == 25.
	 *
	 * Trivially it would be
	 *    if((delta_ps - width_ps) < 200000 || (width_ps < 200000))
	 *               dcr |= FD_DCR_NO_FINE;
	 *
	 * Most likely the calculation below fails with negatives, but
	 * with negative spacing we get no pulses, and fine is irrelevant
	 */

	/*delta.tv_sec = a[FD_ATTR_OUT_DELTA_L];
	delta.tv_nsec = a[FD_ATTR_OUT_DELTA_COARSE] * 8;
	width.tv_sec = ((uint64_t)(a[FD_ATTR_OUT_END_H]) << 32
			| a[FD_ATTR_OUT_END_L])
		- ((uint64_t)(a[FD_ATTR_OUT_START_H]) << 32
		   | a[FD_ATTR_OUT_START_L]);
	if (a[FD_ATTR_OUT_END_COARSE] > a[FD_ATTR_OUT_START_COARSE]) {
		width.tv_nsec = 8 * a[FD_ATTR_OUT_END_COARSE]
			- 8 * a[FD_ATTR_OUT_START_COARSE];
	} else {
		width.tv_sec--;
		width.tv_nsec = NSEC_PER_SEC 
			- 8 * a[FD_ATTR_OUT_START_COARSE]
			+ 8 * a[FD_ATTR_OUT_END_COARSE];		
	}*/
	
	delta.tv_sec =((uint32_t) (pulse->loop.utc));
	delta.tv_nsec = (pulse->loop.coarse) * 8;
	width.tv_sec = (pulse->end.utc) - (pulse->start.utc);
	if ((pulse->end.coarse) > (pulse->start.coarse)) {
		width.tv_nsec = 8 * (pulse->end.coarse)
			- 8 * (pulse->start.coarse);
	} else {
		width.tv_sec--;
		width.tv_nsec = NSEC_PER_SEC 
			- 8 * (pulse->start.coarse)
			+ 8 * (pulse->end.coarse);		
	}

	/* delta = delta - width (i.e.: delta is the low-signal width */
	delta.tv_sec -= width.tv_sec;
	if (delta.tv_nsec > width.tv_nsec) {
		delta.tv_nsec -= width.tv_nsec;
	} else {
		delta.tv_sec--;
		delta.tv_nsec = NSEC_PER_SEC - width.tv_nsec + delta.tv_nsec;
	}
	/* finally check */
	if (width.tv_sec == 0 && width.tv_nsec < 200)
		dcr |= FD_DCR_NO_FINE;
	if (delta.tv_sec == 0 && delta.tv_nsec < 200)
		dcr |= FD_DCR_NO_FINE;
		
	fd_ch_writel(fd, ch, dcr, FD_REG_DCR);
	fd_ch_writel(fd, ch, dcr | FD_DCR_UPDATE, FD_REG_DCR);
		 
	if (mode == FD_OUT_MODE_DELAY)
	    fd_ch_writel(fd, ch, dcr | FD_DCR_ENABLE, FD_REG_DCR);
	else if (mode == FD_OUT_MODE_PULSE)
		fd_ch_writel(fd, ch, dcr | FD_DCR_ENABLE | FD_DCR_PG_ARM,  FD_REG_DCR);
	
	return 0;
}



/* The "pulse_ps" function relies on the previous one **/
int fdelay_config_pulse_ps(struct fd_dev *fd,
			   int channel, struct fdelay_pulse_ps *ps)
{
	struct fdelay_pulse p;

	p.mode = ps->mode;
	p.rep = ps->rep;
	p.start = ps->start;
	p.end = ps->start;
	fdelay_add_ps(&p.end, ps->length);
	fdelay_pico_to_time(&ps->period, &p.loop);
	return fdelay_config_pulse(fd, channel, &p);
}

int fdelay_get_config_pulse(struct fd_dev *fd, int channel, struct fdelay_pulse *pulse)
{
	uint32_t utc_h, utc_l;
	uint32_t input_offset, output_offset, output_user_offset;

	memset(pulse, 0, sizeof(struct fdelay_pulse));

	uint32_t dcr = fd_ch_readl(fd, channel, FD_REG_DCR);
	if(! (dcr & FD_DCR_ENABLE))
		pulse->mode = FD_OUT_MODE_DISABLED;
	else if(dcr & FD_DCR_MODE)
		pulse->mode = FD_OUT_MODE_PULSE;
	else
		pulse->mode = FD_OUT_MODE_DELAY;	

	uint32_t rcr = fd_ch_readl(fd, channel, FD_REG_RCR);
	if(rcr & FD_RCR_CONT)
		pulse->rep = 0xffffffff;
	else
		pulse->rep = FD_RCR_REP_CNT_R(rcr) + 1;
	
	utc_h = fd_ch_readl(fd, channel, FD_REG_U_STARTH);
	utc_l = fd_ch_readl(fd, channel, FD_REG_U_STARTL);
	pulse->start.utc = (((uint64_t)utc_h) << 32) | utc_l;

	
	pulse->start.coarse = fd_ch_readl(fd, channel, FD_REG_C_START);

	pulse->start.frac = fd_ch_readl(fd, channel, FD_REG_F_START);

	utc_h = fd_ch_readl(fd, channel, FD_REG_U_ENDH);
	utc_l = fd_ch_readl(fd, channel, FD_REG_U_ENDL);
	pulse->end.utc = (((uint64_t)utc_h) << 32) | utc_l;
	
	
	pulse->end.coarse = fd_ch_readl(fd, channel, FD_REG_C_END);

	pulse->end.frac = fd_ch_readl(fd, channel, FD_REG_F_END);
	
	utc_l = fd_ch_readl(fd, channel, FD_REG_U_DELTA);
	pulse->loop.utc = utc_l;
	
	pulse->loop.coarse = fd_ch_readl(fd, channel, FD_REG_C_DELTA);

	pulse->loop.frac = fd_ch_readl(fd, channel, FD_REG_F_DELTA);
	
	int m = pulse->mode & 0x7f;
	output_offset = fd->calib.zero_offset[channel];
	output_user_offset = fd->ch_user_offset [channel];
	input_offset = fd->calib.tdc_zero_offset;
	
	switch(m)
	{
		case FD_OUT_MODE_DISABLED:
		/* hack for Steen/COHAL: if channel is disabled, apply delay-mode offsets */
		case FD_OUT_MODE_DELAY:
			fdelay_add_signed_ps(&pulse->start, -(signed)output_offset);
			fdelay_add_signed_ps(&pulse->end, -(signed)output_offset);
			fdelay_add_signed_ps(&pulse->start, -(signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->end, -(signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->start, -(signed)input_offset);
			fdelay_add_signed_ps(&pulse->end, -(signed)input_offset);
		break;
		case FD_OUT_MODE_PULSE:
			fdelay_add_signed_ps(&pulse->start, -(signed)output_offset);
			fdelay_add_signed_ps(&pulse->end, -(signed)output_offset);
			fdelay_add_signed_ps(&pulse->start, -(signed)output_user_offset);
			fdelay_add_signed_ps(&pulse->end, -(signed)output_user_offset);
		break;
	}

	return 0;
}

static void fdelay_subtract_ps(struct fdelay_time *t2,
				   struct fdelay_time *t1, int64_t *pico)
{
	uint64_t pico1, pico2;

	fdelay_time_to_pico(t2, &pico2);
	fdelay_time_to_pico(t1, &pico1);
	*pico = (int64_t)pico2 - pico1;
}

int fdelay_get_config_pulse_ps(struct fd_dev *fd, int channel, 
                               struct fdelay_pulse_ps *ps)
{
	struct fdelay_pulse pulse;

	fdelay_get_config_pulse(fd, channel, &pulse);
		return -1;
	
	memset(ps, 0, sizeof(struct fdelay_pulse_ps));
	ps->mode = pulse.mode;
	ps->rep = pulse.rep;
	ps->start = pulse.start;
	/* FIXME: subtraction can be < 0 */
	fdelay_subtract_ps(&pulse.end, &pulse.start, (int64_t *)&ps->length);
	fdelay_time_to_pico(&pulse.loop, &ps->period);

	return 0;
}


