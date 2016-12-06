#include <iostream>
#include <cstring>
#include <random>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "functions.h"

using namespace std;

void rnd_str(char save_name[]){

	int len = 10;
	char charset[] = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	random_device rnd;
	mt19937 engine(rnd());
	uniform_int_distribution <int> dist(0, strlen(charset) - 1);

	for (int i = 0; i < len; i++){
		save_name[i] = charset[dist(engine)];
	}
	save_name[len-1] = '_';
	save_name[len] = 0;
}

int file_exists(char file_path[]){

	struct stat st;
	return stat(file_path, &st);
}

uint64_t filesize(char file_path[]){
   
	struct stat st;
	stat(file_path, &st);

	uint64_t file_size = st.st_size;
	return file_size;
}

int send_file(int &my_socket, char my_buffer[], uint16_t my_buffer_size){
	
	FILE * file;
	char file_path[256], file_name[256] = {0};
	int16_t socket_data, file_data;
	uint64_t sent_data = 0, file_size;
	
	//Ask for a file
	cout << "Choose a file: ";
	cin.getline(file_path, sizeof(file_path));

	file = fopen(file_path, "r");
	if (file == 0){
		cerr << "Unable to open file" << endl;
		return 1;
	}

	//Find file size from file path
	file_size = filesize(file_path);

	//Find file name from file path
	strncat(file_name, basename(file_path), sizeof(file_name) - 1);

	cout << file_name << " is " << file_size << " bytes" << endl;

	//Send file name and file size to server
	socket_write(my_socket, file_name, sizeof(file_name));
	socket_write(my_socket, &file_size, sizeof(file_size));

	//Send actual data
	while (sent_data < file_size){

		file_data = fread(my_buffer, 1, my_buffer_size, file);
		socket_data = write(my_socket, my_buffer, file_data);

		if (file_data != socket_data){
			cerr << endl << "Size mismatch" << endl;
			return 1;
		}

		sent_data += socket_data;
		
		cout << "Sending data... " << sent_data << "/" << file_size << "\r" << flush;
	}

	if (sent_data == file_size){
		cout << endl << "Data sent" << endl;
	} else {
		cerr << endl << "Failed to send" << endl;
	}

	fclose(file);
	close(my_socket);

	return 0;
}

int receive_file(int &my_socket, char my_buffer[], uint16_t my_buffer_size){

	FILE * file;
	char file_name[256], save_name[256+10] = {0};
	int16_t socket_data, file_data;
	uint64_t received_data = 0, file_size;

	//Receive file name and file size from server
	socket_read(my_socket, file_name, sizeof(file_name));
	socket_read(my_socket, &file_size, sizeof(file_size));

	cout << file_name << " is " << file_size << " bytes" << endl;

	//If file name exists generate a random string
	if (file_exists(file_name) == 0) rnd_str(save_name);
	
	//Append file name to save name
	strncat(save_name, file_name, sizeof(save_name) - strlen(save_name) - 1);
	
	file = fopen(save_name, "w");
	if (file == 0){
		cerr << "Unable to open file" << endl;
		return 1;
	}

	//Receive actual data
	while (received_data < file_size){

		socket_data = read(my_socket, my_buffer, my_buffer_size);
		file_data = fwrite(my_buffer, 1, socket_data, file);

		if (socket_data != file_data){
			cerr << endl << "Size mismatch" << endl;
			return 1;
		}

		received_data += file_data;
			
		cout << "Receiving data... " << received_data << "/" << file_size << "\r" << flush;
	}

	if (received_data == file_size){
		cout << endl << "Data received" << endl;
	} else {
		cerr << endl << "Failed to receive" << endl;
	}

	fclose(file);
	close(my_socket);

	return 0;
}
