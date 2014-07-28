#include <string.h>
#include <dbg.h>
#include <opt.h>
#include "shell.h"
#include "sdb.h"

#define MAX_SDB_DEVS 15
struct sdb_component devs[MAX_SDB_DEVS];

unsigned int base = 0x0;

static void help(void)
{
	mprintf("Use: \"dbgmem [-b][base] [-i][dev_index] <offset> [value]\"\n"
	        "     -b         : Show the base address\n"
	        "     -bbase     : Set a new base address\n"
	        "     -i         : Device/index list\n"
	        "     -idev_index: Set dev_index address as base addres\n"
	        "     offset     : Address to acces, if -bbase is not specified\n"
		    "                : If specfied, the offset respect such address\n"
		    "     value      : Value to write\n");
}

static int cmd_dbgmem(const char *argv[])
{
	unsigned int call_help=0, uarg[2] = {0, 0};
	unsigned int *addr = 0x0;
	int opt, argc = str_vector_length (argv);
	char *tst;
	int index;
	int i;
	uint32_t * v;
	char * name;
	unsigned int n_devs = 0;
	int not_listed = 0, do_write = 0;
	
	optind=0;
	
	while ((opt = getopt(argc, argv, "b::i::h")) != -1) 
	{
		switch(opt)
		{
			case 'b':
				if (optarg)
				{
					base = strtol(optarg, &tst, 16);
					if(tst && *tst)
					{
						kernel_dev(0, "\"%s\" is not an hex number.\n", optarg);
						return 0;
					}
						
				}
				
				mprintf("Base address set to 0x%08x\n", base);
				if(argc <= optind)
					return 0;
				break;
			case 'i':
				not_listed = sdb_get_devices(devs,&n_devs,MAX_SDB_DEVS);
				if(not_listed < 0) 
				{
					if (not_listed)
						kernel_dev(1, "%i devices not listed.\n");
						
					if (optarg)
					{
						index = strtol(optarg, &tst, 16);
						if(tst && *tst)
						{
							kernel_dev(0, "\"%s\" is not an hex number.\n",
							                                           optarg);
							return 0;
						}
						n_devs = index + 1;
					}
					else
						index = 0;
					
					for(i = index; i < n_devs; i++) 
					{
						name = (char *) devs[i].product.name;
						v = (uint32_t *) &(devs[i].addr_first);
						v++;
						if (!optarg)
							mprintf("index %02i: %s - 0x%x\n", i, name,*v);
						else
						{
							base = *v;
							mprintf("Base address set to 0x%08x\n", base);
						}
					}
					if (argc <= optind)
						return 0;
				}
				else
				{
					kernel_dev(0, "Devices not founded.\n");
					return 0;
				}
				break;
			case 'h':
				call_help=1;
				break;
		}
	}

	if (optind  >= argc || optind < argc - 2 || call_help)
	{
		help();
		return 0;
	}
	
	if(base)
		kernel_dev(2, "Base address was set to 0x%08x.\n", base);
		
	do_write = (optind == argc - 2);


	for(i=0; i<= do_write; i++)
	{
		uarg[i] = strtol(argv[optind+i], &tst, 16);
			if(tst && *tst)
			{
				kernel_dev(0, "\"%s\" is not an hex number\n.", argv[optind+i]);
				return 0;
			}
		
		addr = (int *) (base + uarg[0]);
		mprintf("*%08x = %08x\n", (unsigned int) addr, *addr);
		if (do_write){
			*addr = uarg[1];
			mprintf("*%08x = %08x (new value)\n", (unsigned int) addr, *addr);
		}
	}
	
	return 0;
}

DEFINE_WRC_COMMAND(wbr) = {
	.name = "dbgmem",
	.exec = cmd_dbgmem,
};
