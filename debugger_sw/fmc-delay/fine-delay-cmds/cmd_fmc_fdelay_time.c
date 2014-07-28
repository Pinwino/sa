#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fdelay-lib.h"
#include "hw/fd_main_regs.h"

#include <shell.h>
#include <opt.h>
#include "tools-common.h"
#include <linux/jiffies.h>

int fd_wr_mode(struct fd_dev *fd, int on)
{
	uint32_t tcr = fd_readl(fd, FD_REG_TCR);

	if (on) {
		fd_writel(fd, FD_TCR_WR_ENABLE, FD_REG_TCR);
		set_bit(FD_FLAG_WR_MODE, fd->flags);
	} else {
		fd_writel(fd, 0, FD_REG_TCR);
		clear_bit(FD_FLAG_WR_MODE, fd->flags);
		/* not white-rabbit: write default to DAC for VCXO */
		fd_spi_xfer(fd, FD_CS_DAC, 24,
			    fd->calib.vcxo_default_tune & 0xffff, NULL);
	}
	
	if(! (tcr & FD_TCR_WR_PRESENT)){
		return -EOPNOTSUPP;
	}
	else if( ! (tcr & FD_TCR_WR_LINK)){
		return -ENOLINK;
	}
	else
		return 0;
}

int fd_wr_query(struct fd_dev *fd)
{
	int ena = test_bit(FD_FLAG_WR_MODE, fd->flags);

	if (!ena)
		return -ENODEV;
	if (! (fd_readl(fd, FD_REG_TCR) & FD_TCR_WR_LINK))
		return -ENOLINK;
	if (fd_readl(fd, FD_REG_TCR) & FD_TCR_WR_LOCKED){
		return 0;
	}
	return -EAGAIN;
}


int main_time(const char **argv)
{
	struct fdelay_time t;
	int i, err, get = 0, wr_on = 0, wr_off = 0, argc=0;
	char *s;
	optind = 0;

	argc=str_vector_length(argv);

	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv))
		help(FD_CMD_TIME_HELP);


	/* Parse the mandatory extra argument */
	if (optind != argc - 1 || !strcmp (argv[optind], "-h")){
		help(FD_CMD_TIME_HELP);
		return 0;
	}

	s = argv[optind];
	/* Crappy parser */
	if (!strcmp(s, "get"))
		get = 1;
	else if (!strcmp(s, "wr"))
		wr_on = 1;
	else if (!strcmp(s, "local"))
		wr_off = 1;
	else {
		unsigned long long nano;
		unsigned long long sec;

		memset(&t, 0, sizeof(t));
		i = sscanf_addhoc_replacement(s, &sec, &nano);
		if (i < 1) {
			mprintf("time: Not a number \"%s\"\n", s);
			return 0;
		}
		t.utc = sec;
		t.coarse = nano * 1000 * 1000 * 1000 / 8;
	}

	if (get) {
		if ((err = fd_time_get(&fd, &t, NULL)) < 0) {
			mprintf("time: fdelay_get_time(): %s\n", strerror(err));
			return -1;
		}

		err = fd_wr_query(&fd);
		mprintf("WR Status: ");
		switch(err)
		{
			case -ENODEV: 	mprintf("disabled.\n"); break;
			case -ENOLINK: 	mprintf("link down.\n"); break;
			case -EAGAIN: 	mprintf("synchronization in progress.\n"); break;
			case 0: 	mprintf("synchronized.\n"); break;
			default:   	mprintf("error: %s\n", strerror(errno)); break;
		}

		//When 64 bit pritting is supported
		//mprintf("Time: %i.%09i\n", (long long)t.utc, (long)t.coarse * 8); 
		
		//Until that glorious day
		mprintf("Time: ");
		mprint_64bit (t.utc);
		mprintf(".%09i\n", t.coarse * 8);
		
		
		return 0;
	}


	if (wr_on) {
		printf("Locking the card to WR: ");
		
		err = fd_wr_mode(&fd, 1);
		
		if(err == -ENOTSUP)
		{
			mprintf("time: no support for White Rabbit (check the gateware).\n");
			return 0;
		} else if (err) {
			mprintf("time: fdelay_wr_mode(): %s\n", strerror(err));
			return 0;
		}
		
		int j = jiffies + 1000 * 90;
		while ((err =  fd_wr_query(&fd)) != 0) {
			if( err == -ENOLINK )
			{
				mprintf("time: no White Rabbit link (check the cable and the switch).\n");
				return 0;
			}
			printf(".");
			usleep(1000*1000);
			
			if (jiffies>j) {
				fd_wr_mode(&fd, 0);
				mprintf("\ntime: fdelay_wr_mode(): %s\n", strerror(ETIME));
				return 0;
			}
		}
		mprintf(" locked!\n");
		return 0;
	}

	if (wr_off) {
		if ((err=fd_wr_mode(&fd, 0)) < 0) {
			mprintf("time: fdelay_wr_mode(): %s\n", strerror(err));
		}
		return 0;
	}

	if (test_bit(FD_FLAG_WR_MODE, fd.flags)){
		mprintf("time: fdelay_set_time(): %s\n", strerror(EAGAIN));
		return 0;
	}
	else if ((err=fd_time_set(&fd, &t, NULL)) < 0) {
		mprintf("time: fdelay_set_time(): %s\n", strerror(err));
		return 0;
	}
	/* wtf */
	fd_writel(&fd, 0, FD_REG_TCR);
	return 0;
}

DEFINE_WRC_COMMAND(time) = {
	.name = "time",
	.exec = main_time,
};
