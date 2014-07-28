/**
 ******************************************************************************* 
 * @file cmd_spec.cpp
 *  @brief SPEC command terminal
 *
 *  Copyright (C) 2013
 *
 *  @author Miguel Jimenez Lopez <klyone@ugr.es>
 *
 *  @bug ---
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
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
	
	for(it = itb ; it != ite ; it++) {
		cout << *it <<endl;
	}
}

int main ()
{
	Vuart vuart_wrpc("./caloe/devices/vuart/vuart.cfg");
	Vuart vuart("./vuart-dbg.cfg");
  
	string ip;
	long int len_pulse;
	timespec t_trig;
	int ch;
	char mode;
	char specific_spec;
	bool specific_spec_b = false;
	string cmd;
	string virtual_cmd;
	string proto("udp");
	
	cout <<endl<<"----------------------------------------------------"<<endl;
	      cout <<" Fine Delay Stand Alone Mode - Remote Configuration "<<endl;
		  cout <<"----------------------------------------------------"<<endl;



		specific_spec_b = true;
		ip = get_ip();
		cmd ="vuart";
			
	  
		if (cmd == "exit"){}
		else {								  
			if(cmd == "vuart") {
				string res;
				char trash;
				char loop='y';
																	
				cout <<endl<< "Warning: Vuart is under testing!!"<<endl;
				if(!specific_spec_b) {
					ip = get_ip();
				}

				cout << "Note: Use \"exit\" to quit the applicattion"<<endl<<endl;
																	
				trash = getchar();
															
				do {
					cout << "vuart# ";
					getline(cin,virtual_cmd);
														
					if(virtual_cmd == "exit")
						loop = 'n';
									
					else {
																			
						if(HIDE_GUI_STAT_CONT == 1) {
							if(virtual_cmd == "gui" || virtual_cmd == "stat cont") {
								cout << "Warning: gui and stat cont must not be used in remote mode (changing to stat cmd)!!"<<endl;
								virtual_cmd = "stat";
							}
						}
																		
						vuart.flush(proto+"/"+ip);
																			
						if(virtual_cmd == "gui" || virtual_cmd == "stat") {
							vuart.execute_cmd(proto+"/"+ip,"refresh 0",1);
							res = vuart.execute_cmd(proto+"/"+ip,virtual_cmd,1);
							vuart.execute_cmd(proto+"/"+ip,"refresh 1",1);
						}
						else
							res = vuart.execute_cmd(proto+"/"+ip,virtual_cmd,3);
																			
																	
						printf_vuart(res,virtual_cmd);

						}
																	
					} while(loop == 'y');
				}
				else {
					cout <<endl<<endl<<cmd<<": Unrecognized command"<<endl<<endl;
				}
			}
  
	return 0;
}
