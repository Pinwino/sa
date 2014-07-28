/*
 * The "official" fine-delay API
 *
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2 as published by the Free Software Foundation or, at your
 * option, any later version.
 */
#ifndef __FDELAY_H__
#define __FDELAY_H__

#include <stdint.h>
#include "fine-delay.h"
#include <linux/math64.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Convenience macro for converting the physical output connector
   numbers (as seen on the mezzanine's front panel) to convention used
   by the drive (0..3). We keep 0..3 indexing to maintain library
   compatibility. */
#define FDELAY_OUTPUT_HW_TO_USER(out) ((out) + 1)
#define FDELAY_OUTPUT_USER_TO_HW(out) ((out) - 1)

extern struct fd_dev fd;

struct fdelay_time {
	uint64_t utc;
	uint32_t coarse;
	uint32_t frac;
	uint32_t seq_id;
	uint32_t channel;
};

/* The structure used for pulse generation */
struct fdelay_pulse {
	/* FD_OUT_MODE_DISABLED, FD_OUT_MODE_DELAY, FD_OUT_MODE_PULSE */
	int mode;
	/* -1 == infinite */
	int rep;

	struct fdelay_time start;
	struct fdelay_time end;
	struct fdelay_time loop;
};

/* An alternative structure, internally converted to the previous one */
struct fdelay_pulse_ps {
	int mode;
	int rep;
	struct fdelay_time start;
	uint64_t length;
	uint64_t period;
};


extern void fdelay_pico_to_time(uint64_t *pico, struct fdelay_time *time);
extern void fdelay_time_to_pico(struct fdelay_time *time, uint64_t *pico);

extern int fdelay_config_pulse(struct fd_dev *fd,
                  int channel, struct fdelay_pulse *pulse);
extern int fdelay_config_pulse_ps(struct fd_dev *fd,
				  int channel, struct fdelay_pulse_ps *ps);
extern int fdelay_get_config_pulse_ps(struct fd_dev *fd,
			      int channel, struct fdelay_pulse_ps *ps);


#endif /* __FDELAY_H__ */
