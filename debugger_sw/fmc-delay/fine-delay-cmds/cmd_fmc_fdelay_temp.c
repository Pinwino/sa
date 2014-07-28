#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "fdelay-lib.h"

#include <shell.h>

#include "tools-common.h"

int main_temp(const char **argv)
{

	if (str_vector_length(argv) != 0){
		help(FD_CMD_TEMP_HELP);
		return 0;
	}
	
	int temp = fd_read_temp(&fd, 0);
	mprintf("Temperature %i.%03i\n", temp / 16, (temp & 0xf) * 1000 / 16);
	return 0;	
}

DEFINE_WRC_COMMAND(temp) = {
	.name = "temp",
	.exec = main_temp,
};
