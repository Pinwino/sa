/* Simple demo that acts on the termination of the first board */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fdelay-lib.h"
#include <opt.h>

#include "tools-common.h"
#include <shell.h>
	     
int main_term (const char **argv)
{
	int newval, argc = 0;
	optind = 0;

	argc=str_vector_length(argv);

	/* Standard part of the file (repeated code) */
	if (tools_need_help(argc, argv)){
		help(FD_CMD_TERM_HELP);
		return 0;
	}

	newval = -1;
	
	if (optind + 1 == argc) {
		char *s = argv[optind];
		if (!strcmp(s, "0") || !strcmp(s, "off"))
		    newval = 0;
		else if (!strcmp(s, "1") || !strcmp(s, "on"))
			newval = 1;
		else{
			help(FD_CMD_TERM_HELP);
			return 0;
		}
	}
	
	int err = 0;

	switch(newval) {
	case 1:
		fd.tdc_flags |= FD_TDCF_TERM_50;
		fd_gpio_set(&fd, FD_GPIO_TERM_EN);
		break;
	case 0:
		fd.tdc_flags &= ~FD_TDCF_TERM_50;
		fd_gpio_clr(&fd, FD_GPIO_TERM_EN);
		break;
	}
	
	if (err)
	{
		mprintf("term: error setting termination: %s", strerror(err));
		return 0;
	}

	printf("termination is %s\n",
	       fd.tdc_flags & FD_TDCF_TERM_50 ? "on" : "off");

	return 0;
}

DEFINE_WRC_COMMAND(term) = {
	.name = "term",
	.exec = main_term,
};
