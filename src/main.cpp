/**
 * @author  Mitchel Taylor
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 * 		Sources
 https://linux.die.net
 */
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "../include/global.h"
#include "../include/server.h"
#include "../include/client.h"
#include "../include/util.h"

using namespace std;


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

int main(int argc, char** argv)
{

	util util;
	if(argc < 3){
		//Do not start, not enough arguments
		cout << "USAGE: <progname> <server or client> <portnum>\n";
		cout << "Example: ./a s 5001\n";

	}else{
		if(strcmp(argv[1],"c") == 0 && util.isValidPort(argv[2]) == true){
				client c;
				c.startClient(argv[2]);
		}else if(strcmp(argv[1],"s") == 0 && util.isValidPort(argv[2]) == true){
				server s;
				s.startServer(argv[2]);
		}else{
			// Do not start, either port or s or c is incorrect
			cout << "USAGE: <progname> <server or client> <portnum>\n";
			cout << "Example: ./a s 5001\n";
		}
	}

	return 0;
}
