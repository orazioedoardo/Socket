#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "functions.h"

using namespace std;

void connect_to_server(int &my_socket, sockaddr_in &my_server){
	
	if (connect(my_socket, (sockaddr *) &my_server, sizeof(my_server)) < 0){
		cerr << "Connection failed" << endl;
		exit(1);
	} else {
		cout << "Connection established" << endl;
	}
}

void create_socket(int &my_listener){
	
	if ((my_listener = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cerr << "Unable to create socket" << endl;
		exit(1);
	} else {
		cout << "Successfully created socket" << endl;
	}
}

void bind_port(int &my_listener, sockaddr_in &my_server){
	
	if (::bind(my_listener, (sockaddr *) &my_server, sizeof(my_server)) < 0){
		cerr << "Binding failed" << endl;
		exit(1);
	} else {
		cout << "Binding successful" << endl;
	}
}

void listen_for_client(int &my_listener){
	
	if (listen(my_listener, 1) < 0){
		cerr << "Failed to listen" << endl;
		exit(1);
	} else {
		cout << "Listening..." << endl;
	}
}

void accept_client(int &my_socket, int &my_listener, sockaddr_in &my_client){
	
	socklen_t my_client_size = sizeof(my_client);
	if ((my_socket = accept(my_listener, (sockaddr *) &my_client, &my_client_size)) < 0){
		cerr << "Connection failed" << endl;
		exit(1);
	} else {
		cout << "Client " << inet_ntoa(my_client.sin_addr) << " connected" << endl;
	}
}

void socket_read(int &my_socket, char my_data[], size_t my_data_size){
	
	if (read(my_socket, my_data, my_data_size) <= 0){
		cerr << "Read error" << endl;
		exit(1);
	}
}

void socket_write(int &my_socket, char my_data[], size_t my_data_size){
	
	if (write(my_socket, my_data, my_data_size) <= 0){
		cerr << "Write error" << endl;
		exit(1);
	}
}
