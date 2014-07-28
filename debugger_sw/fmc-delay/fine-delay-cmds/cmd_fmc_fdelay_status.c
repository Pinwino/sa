#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fdelay-lib.h"

#include <shell.h>

#include "tools-common.h"
#include "opt.h"



int main_status(const char **argv)
{
	struct fdelay_pulse p;
	int  ch, raw = 0, opt, argc;
	optind=0;
	
	argc = str_vector_length (argv);

	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv)){
		help(FD_CMD_STAT_HELP);
		return 0;
	}

	while ((opt = getopt(argc, argv, "rh")) != -1) {
		switch (opt) {
		case 'r':
			raw = 1;
			break;
		case 'h':
			help(FD_CMD_STAT_HELP);
			return 0;
		}
	}


	for (ch = 1; ch <= 4; ch++) {
		if (fdelay_get_config_pulse(&fd, FDELAY_OUTPUT_USER_TO_HW(ch), &p) <0){
			mprintf("status: get_config(channel %i): %s\n", ch, strerror(errno));
		}
		/* pass hw number again, as the function is low-level */
		report_output_config(FDELAY_OUTPUT_USER_TO_HW(ch),
				    &p, raw ? TOOLS_UMODE_RAW : TOOLS_UMODE_USER);
	}
	return 0;
}

DEFINE_WRC_COMMAND(status) = {
	.name = "status",
	.exec = main_status,
};
