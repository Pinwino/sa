/*
 * This work is part of the FMC DEL 1ns 4cha - stand-alone application
 * 
 * Author: Jose Jimenez
 * 
 * Simple tool for loading wb-debbugger RAM using Etherbone.
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <caloe/lib/access_internals.h>
#include <errno.h>
#include <unistd.h>
//caloe
#define is_config_int 	0
#define nbytes 			100
#define line_end 		0


//ethb-cl
#define def_init_offset 0x40000
#define conf_buf 		256
#define def_ethb_conf 	"udp/10.10.1.1"

static void help(char *name)
{
	fprintf(stderr, 
	"Use: \"%s [-a base address ] [-e proto/IPv4] <ram file>\"\n", name);
	fprintf(stderr, 
	"   -a: RAM base address (default 0x%x)\n", def_init_offset);
	fprintf(stderr, 
    "   -e: Transport protocol [udp|tcp]/device IPv4 (default %s)\n", def_ethb_conf);
	fprintf(stderr, 
	"   <ram file>: Path to the .ram file to be loaded\n");
	
	fprintf(stderr, "\nReport bugs to <fmc-delay-1ns-8cha-sa@ohwr.org>\n");

	exit(1);
}

static int checkIP(char *ip)
{
    struct sockaddr_in sock;
    int ret = inet_pton(AF_INET, ip, &(sock.sin_addr));
    
    return (ret != 1);
}

int main (int argc, char ** argv)
{
	access_caloe access;
	network_connection nc;
	int c, i;
	unsigned int base = def_init_offset;
	FILE *fp;
	
	uint32_t cntr = 0;
	
   	char *ptr;
   	char *s_val[3];
   	char *line = (char *) malloc (nbytes + 1);

	char *ip = (char *) malloc(conf_buf);
	uint32_t u_val[2];
	
	strcpy(ip, def_ethb_conf);
	
	while ((c = getopt (argc, argv, "a:e:h")) != -1)
	{
		char aux[conf_buf];
		
		switch(c)
		{
		case 'a':
			base = (unsigned int)strtol(optarg, &ptr, 16);
			if (ptr && *ptr)
			{
				fprintf(stderr, "%s: \"%s\" is not an hex number\n",
				              argv[0], optarg);
				exit(1);
			}
		break;
		case 'e':
			strcpy(aux, optarg);
			ptr=strpbrk(aux, "/");
			*ptr=0;
			if (!strcmp(aux, "tpc"))
			{
				fprintf(stderr, "%s: Etherbone does not support tcp\n",
				              argv[0]);
				exit(1);
			}
			else if (strcmp(aux, "udp"))
			{
				fprintf(stderr, "%s: invalid protocol\"%s\"\n",
				              argv[0], aux);
				exit(1);
			}
			else if (checkIP(ptr+1))
			{
				fprintf(stderr, "%s: \"%s\" is not a valid IPv4 address\n",
				              argv[0], ptr+1);
				exit(1);
			}
			else
				strcpy(ip, optarg);
		break;
		case 'h':
		default:
			help(argv[0]);
		}
	}
	
	if (optind >= argc) 
	{
		fprintf(stderr, "%s: Expected new ram file after options.\n",
			argv[0]);
		exit(1);
	}
	
	printf(
	"WARNING: RAM located at 0x%x will be overwritten with %s (acces by %s)\n",
	          base, argv[optind], ip);
	printf("Do you want to proceed? (y/n): ");
	char proceed;
	for(;;)
	{
		scanf("%s", &proceed);
		if (proceed == 'y')
		{
			printf("Memory overwrite will proceed, please wait... \n");
			break;
		}
		else if (proceed == 'n')
		{
			printf("Don't panic. Overwrite aborted!\n");
			return 0;
		}
		else 
			printf("Invalid option, please enter [y|n]: ");
	}

	// Build an network_connection struct of access_internals
	build_network_con_caloe (ip, &nc);
	
	if ((fp = fopen(argv[optind], "r")) == NULL)
	{
		fprintf(stderr, "    %s: ERROR while opening %s file: %s\n",
		             argv[0], argv[optind], strerror(errno));
		exit(1);
	}

		while (fgets(line, nbytes, fp) != NULL) {
			s_val[0] = line; /* wtf? why only works this way? */
			
			/* parse chain */
			for (i=1; i<=2; i++)
			{
				s_val[i]=strpbrk(s_val[i-1], " ");
				*s_val[i]=line_end;
				s_val[i]++;
			}
			
			/* eliminate /n */
			s_val[0]=strpbrk(s_val[2], "\n");
			*s_val[0]=line_end;

			for (i=0; i<=1; i++)
			{
				u_val[i] = (uint32_t)strtol(s_val[i+1], &ptr, 16)*(4-3*i);
				if (ptr && *ptr)
				{
					fprintf(stderr,
				    "    %s: ERROR \"%s\" is not an hex number\n",
				              argv[0], s_val[i]);
					exit(1);
				}
			}
		cntr++;

		build_access_caloe(base, u_val[0], u_val[1], 0, MASK_OR, is_config_int,
		                      WRITE, SIZE_4B, &nc, &access);
		if ((execute_caloe(&access)) < 0)
			exit(1);
	
		if ((uint32_t) access.value != u_val[1])
		{
			fprintf(stderr,
			        "    %s: ERROR overwritting RAM: access %0x%08x (%u), \n",
			               argv[0], cntr, cntr);
			fprintf(stderr,
			        "        Values to load from file: offset %s, value to write %s\n",
			              s_val[0], s_val[1]);
			fprintf(stderr,
			        "        Converted values: offset %08x, value %08x\n",
			              u_val[0], u_val[1]);
			fprintf(stderr,
			        "        Written values: offset %08x, value %08x\n",
			              u_val[1], access.value);
			fprintf(stderr,
		            "            Difference (written value - read value): %d\n",
		                 (u_val[1] - (uint32_t) access.value));

			exit(1);
		}
	}
	
	fclose(fp);
	// Free access_caloe and network_connection memory
	free_access_caloe(&access);
	
	printf("\nRAM successfully overwrited!\n");
	
	return 0;
}
