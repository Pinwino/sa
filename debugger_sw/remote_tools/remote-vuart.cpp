/*
 * This work is part of the White Rabbit project
 * 
 * Jose Jimenez  <jjimenez.wr@gmail.com>, Copyright (C) 2014.
 * Released according to the GNU GPL version 3 (GPLv3) or later.
 * 
 * Tool for virtual UART remote acces by CALoE (Etherbone).
 * 
 * Inspiered by:
 * * 
 * * @file cmd_spec.cpp
 * *  @brief SPEC command terminal
 * *
 * *  Copyright (C) 2013
 * *
 * * @author Miguel Jimenez Lopez <klyone@ugr.es>
 * *
 * 
 * Modifications:
 * - Main function simplification
 * - Absolute path building mechainsm for *.cfg files instead of relative path 
 *   for folder independence calling.
 */
 
#include "../devices/dio/Dio.h"
#include "../devices/vuart/Vuart.h"

#include <arpa/inet.h>

using namespace std;

#define HIDE_GUI_STAT_CONT 0

bool isValidIpAddress(const char *ipAddress)
{
	char buff[30];

	strcpy(buff,ipAddress);

	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, buff, &(sa.sin_addr));
	return result != 0;
}

string get_ip() {
	string ip;
	do {
		cout << "Node IP: ";
		cin >> ip;

		if(cin.fail()) {
			cout <<endl<<"Error: CIN istream error!"<<endl;
			exit(-1);
		}
	} while(!isValidIpAddress(ip.c_str()));

	return ip;
}

char get_vuart_loop_mode() {
	char loop;

	do {
		cout << "Loop mode (y/n)?: ";
		cin >> loop;

		if(cin.fail()) {
			cout <<endl<<"Error: CIN istream error!"<<endl;
			exit(-1);
		}
	} while(loop != 'y' && loop != 'n');

	return loop;
}

// Remove additional characters
string delete_format_chars(string s) {
	string s2;
	int size = s.length();
	int i;
	
	for(i = 4 ; i < size ; i = i+5) {
		s2.push_back(s.at(i));
	}
	
	return s2;
}

// Print vuart command response
void printf_vuart(string res,string cmd) {
	vector<string> res_split = split(res,'\n');
	vector<string>::iterator it;
	vector<string>::iterator itb = res_split.begin();
	vector<string>::iterator ite = res_split.end()-1;
	string fl = delete_format_chars(*itb);
	
	bool cmd_first_line = (fl.find(cmd) != string::npos);
	
	if(cmd_first_line)
		itb++;
	
	for(it = itb ; it != ite ; it++){
		cout << *it <<endl;
	}
}

int main ()
{
	char *vuart_cfg_path = (char *) malloc(2048*sizeof(char));
	strcpy(vuart_cfg_path, CURPATH);
	vuart_cfg_path = strcat(vuart_cfg_path, "/vuart-dbg.cfg");
	Vuart vuart(vuart_cfg_path);

	string ip;
	string cmd;
	string virtual_cmd;
	string proto("udp");

	cout <<"************************************************************"<<endl;
	cout <<"*    Fine Delay Stand Alone Mode - Remote Configuration    *"<<endl;
	cout <<"*                            by                            *"<<endl;
	cout <<"*                       Jose Jimenez                       *"<<endl;
	cout <<"*                                                          *"<<endl;
	cout <<"*                                                          *"<<endl;
	cout <<"*                       - WARNING -                        *"<<endl;
	cout <<"*      This is a beta version, please report bugs to:      *"<<endl;
	cout <<"*             <fmc-delay-1ns-8cha-sa@ohwr.org>             *"<<endl;
	cout <<"************************************************************"<<endl;
	cout <<endl;

	ip = get_ip();
	cmd ="vuart";

	if (cmd == "exit"){}
	else
	{
		if(cmd == "vuart")
		{
			string res;
			char trash;

			cout <<endl<<"Warning: Vuart is also under testing!!"<<endl;
			cout <<"Note: Use \"exit\" to quit the applicattion"<<endl<<endl;
			cout <<"Note: Type \"help\" to see command list"<<endl;
			cout <<"Note: Use \"<command_name -h>\" to explore commands usage"
				 <<endl<<endl;

			trash = getchar();

			while (1) /* Dedicated to Nolo from this "poor cable-peeler" */
			{
				cout << "vuart# ";
				getline(cin,virtual_cmd);

				if(virtual_cmd == "exit")
					break;

				res = vuart.execute_cmd(proto+"/"+ip,virtual_cmd,3);
				printf_vuart(res,virtual_cmd);
			}
		}
		else
		cout<<endl<<endl<<cmd<<": Unrecognized command"<<endl<<endl;
	}
	return 0;
}
