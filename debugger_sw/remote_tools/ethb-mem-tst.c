/*
 * This work is part of the FMC DEL 1ns 4cha - stand-alone application
 * 
 * Author: Jose Jimenez
 * 
 * Simple tool for testing memory usage usign Etherbone.
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

//caloe
#define is_config_int 	0
#define nbytes 			100
#define line_end 		0


//ethb-mem-tst
#define conf_buf 		256
#define def_ethb_conf 	"udp/10.10.1.1"
#define pattern         0xdeafbeef
#define arguments		3

static void help(char *name)
{
	fprintf(stderr, "Use: "
	"\"%s [-e proto/IPv4] [-y] [-w] <base_address> <num_words> <pattern>\"\n"
	"\n", name);
	fprintf(stderr, 
    "         -e: Transport protocol [udp|tcp]/device IPv4 (default %s)\n",
                                                               def_ethb_conf);
	fprintf(stderr,
	"       [-w]: To write the pattern <pattern> or the default pattern.\n"
	"       [-y]: Assume \"yes\" as answer to prompts. Run non-interactively.\n"
	"   <offset>: Memory base addres.\n"
	"<num_words>: Num of word to be written/tested.\n"
	"  <pattern>: Requires -w\n"
	"             Pattern to be read/written\n"
	"             If none, defalut pattern \"0x%08x\" is used.\n", pattern);

	fprintf(stderr, "\nReport bugs to <fmc-delay-1ns-8cha-sa@ohwr.org>\n\n");

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
	int c, i, do_write = 0, warn = 1, limit=arguments;;                                                                    
	
	uint32_t cntr = 0;


	char *ip = (char *) malloc(conf_buf);
	char *end;
	uint32_t uarg[arguments], _val;
	char *ptr;
	
	strcpy(ip, def_ethb_conf);
	
	while ((c = getopt (argc, argv, "e:ywh")) != -1)
	{
		char aux[conf_buf];
		
		switch(c)
		{
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
		case 'y':
			warn = 0;
			break;
		case 'w':
			do_write=1;
			break;
		case 'h':
			help(argv[0]);
			exit(1);
		}
	}

	if (optind  >= argc || optind < argc - arguments)
		help(argv[0]);

	if(optind != argc - arguments){
		uarg[arguments] = pattern;
		limit--;
	}

	/* convert the trailing hex number or numbers */
	for(i=0; i<limit; i++){
		uarg[i] = strtol(argv[optind+i], &end, abs(16*(1-i)));
		if (end && *end) {
			fprintf(stderr, "%s: \"%s\" is not a number\n",
				argv[0], argv[optind+i]);
			exit(1);
		}
	}

	if (uarg[0] & 3) {
		fprintf(stderr, "%s: address \"%s\" not multiple of 4\n",
			argv[0], argv[optind + 0]);
		exit(1);
	}

	if (warn){
		printf("\n"
		"                             - WARNING -                             "
		"\n");
		printf(
		"*- Access to a non-existent whisbone address will freeze your PC -*\n"
		"    Make sure 0x%08x is a valid address.\n"
		"    Verify that the number of positions to be tested \"%u (0x%x)\"\n"
		"    are within memory boundaries.\n\n",
		uarg[0], uarg[1], uarg[1]);
		printf(
		"*- Writing a non-valid value may cause an unpredictable behaviour -*\n"
		"    Adv: The memory should never be accessed while testing.\n"
		"         Hold WB Masters on reset while reading/writing patterns.\n");
		printf(
		"                             -----------                             "
		"\n\n");
		printf("Continue [y|n]: ");
		char proceed;
		for(;;)
		{
			scanf("%s", &proceed);
			if (proceed == 'y')
				break;
			else if (proceed == 'n')
				return 0;
			else 
				printf("Invalid option, please enter [y|n]: ");
		}
	}

	// Build an network_connection struct of access_internals
	build_network_con_caloe (ip, &nc);
	
	int last = 0, words = 0;
	for(i=uarg[0]; i < uarg[0]+uarg[1]*0x4; i+=0x4){
		if (!do_write) 
		{
			build_access_caloe(0x0, i, _val, 0, MASK_OR, is_config_int,
		                      READ, SIZE_4B, &nc, &access);
			if ((execute_caloe(&access)) < 0)
				exit(1);
			if (access.value == uarg[3]) 
			{
				if(((i-last) == 0x4) || (last == 0))
				{
					if(words == 0)
						printf("Starting at %08x we have ", i);
					words++;
				}
			} 
			else 
			{
				if (words == 1)
					printf("only one word\n");
				else if (words > 1)
				{
					printf("%u words, the last one at %08x\n", words, last);
				}
				words = 0;
			} 
			last=i;
		}
		else{
			build_access_caloe(0x0, i, uarg[3], 0, MASK_OR, is_config_int,
		                      WRITE, SIZE_4B, &nc, &access);
		    if ((execute_caloe(&access)) < 0)
				exit(1);
		}
	}
	
	if (words == 1)
		printf("only one word\n");
	else if (words > 1)
		printf("%u words, the last one at %08x\n", words, last);
	// Free access_caloe and network_connection memory
	free_access_caloe(&access);
	

	return 0;
}
