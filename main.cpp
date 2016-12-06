#include <iostream>
#include <cstring>
#include <limits>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "functions.h"

using namespace std;

int main(int argc, char * argv[]){

	int my_listener, my_socket;
	char my_buffer[1024], option;
	
	sockaddr_in my_server, my_client;

	if (argc < 3){
		cout << "Usage: socket [-c ip_address port]" << endl;
		cout << "              [-l port]" << endl;
		return 1;
	}

	if (strcmp(argv[1], "-c") == 0){

		//Act as a client
		if (atoi(argv[3]) < 1024 || atoi(argv[3]) > 65535){
			cout << "Only ports from 1024 to 65535" << endl;
			return 1;
		}

		//Fill server parameters
		my_server.sin_family = AF_INET;
		my_server.sin_addr.s_addr = inet_addr(argv[2]);
		my_server.sin_port = htons(atoi(argv[3]));

		//Socket creation
		create_socket(my_socket);
		
		//Connect to server
		connect_to_server(my_socket, my_server);
		
		//Ask whether to send or receive
		cout << "Send or receive file? [1/2] ";
		cin >> option;

		//Ignore the new line character left in the stream
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		//Send option to server
		socket_write(my_socket, &option, 1);
		
		//Main routine
		if (option == '1'){
			send_file(my_socket, my_buffer, sizeof(my_buffer));
		} else if (option == '2'){
			receive_file(my_socket, my_buffer, sizeof(my_buffer));
		}

	} else if (strcmp(argv[1], "-l") == 0){

		//Act as a server
		if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535){
			cout << "Only ports from 1024 to 65535" << endl;
			return 1;
		}

		//Fill server parameters
		my_server.sin_family = AF_INET;
		my_server.sin_addr.s_addr = htonl(INADDR_ANY);
		my_server.sin_port = htons(atoi(argv[2]));

		//Socket creation
		create_socket(my_listener);

		//Bind to port
		bind_port(my_listener, my_server);

		//Listen for client
		listen_for_client(my_listener);

		//Accept client
		accept_client(my_socket, my_listener, my_client);

		//Get option from client
		socket_read(my_socket, &option, 1);

		//Main routine
		if (option == '1'){
			receive_file(my_socket, my_buffer, sizeof(my_buffer));
		} else if (option == '2'){
			send_file(my_socket, my_buffer, sizeof(my_buffer));
		}

		close(my_listener);
	}
}
