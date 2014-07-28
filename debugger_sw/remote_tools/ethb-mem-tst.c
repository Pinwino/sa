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

//caloe
#define is_config_int 	0
#define nbytes 			100
#define line_end 		0


//ethb-mem-tst
#define conf_buf 		256
#define def_ethb_conf 	"udp/10.10.1.1"
#define pattern         0xdeafbeef

static void help(char *name)
{
	fprintf(stderr, 
	"Use: \"%s [-e proto/IPv4] [-w] <base_address> <num_words>\"\n", name);
	fprintf(stderr, 
    "   -e: Transport protocol [udp|tcp]/device IPv4 (default %s)\n", def_ethb_conf);
	fprintf(stderr, 
	"   <ram file>: Path to the .ram file to be loaded\n");
	
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
	int c;                                                                    
	
	uint32_t cntr = 0;
	char *line = (char *) malloc (nbytes + 1);
	
	const char tok[] = " ", t[]="/";

	char *ip = (char *) malloc(conf_buf);
	char *end;
	uint32_t uarg[2], _val;
	int i, do_write=0;
	char *ptr;
	
	strcpy(ip, def_ethb_conf);
	
	while ((c = getopt (argc, argv, "e:wh")) != -1)
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
		case 'w':
			do_write = 1;
			break;
		case 'h':
		default:
			help(argv[0]);
			exit(1);
		}
	}

	if (optind  >= argc || optind != argc - 2)
		help(argv[0]);

	/* convert the trailing hex number or numbers */
	for (i=0;i<=1;i++){
		uarg[i] = strtol(argv[optind + i], &end, 16*(1-i));
		if (end && *end) {
			fprintf(stderr, "%s: \"%s\" is not a number\n",
				argv[0], argv[optind + i]);
			exit(1);
		}
	}

	if (uarg[0] & 3) {
		fprintf(stderr, "%s: address \"%s\" not multiple of 4\n",
			argv[0], argv[optind + 0]);
		exit(1);
	}

	// Build an network_connection struct of access_internals
	build_network_con_caloe (ip, &nc);
	
	int last = 0, words = 0;
	for(i=uarg[0]; i < uarg[0]+uarg[1]*0x4; i+=0x4){
	/* by default, operate quietly (only report read value) */
		if (!do_write) 
		{
			build_access_caloe(0x0, i, _val, 0, MASK_OR, is_config_int,
		                      READ, SIZE_4B, &nc, &access);
			if ((execute_caloe(&access)) < 0)
				exit(1);
			if (access.value == pattern) 
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
			build_access_caloe(0x0, i, pattern, 0, MASK_OR, is_config_int,
		                      WRITE, SIZE_4B, &nc, &access);
		    if ((execute_caloe(&access)) < 0)
				exit(1);                  
		    printf("%08x %08x\n", i, access.value);
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
