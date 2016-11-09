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

size_t filesize(char file_path[]){
   
	struct stat st;
	stat(file_path, &st);

	size_t num_size = st.st_size;
	return num_size;
}

int send_file(int &my_socket, char my_buffer[], size_t my_buffer_size){
	
	FILE * file;
	char file_path[256], file_name[256] = {0}, file_size[16];
	size_t current_data, written_data = 0, num_size;
	
	//Ask for a file
	cout << "Choose a file: ";
	cin.getline(file_path, sizeof(file_path));

	file = fopen(file_path, "r");
	if (file == 0){
		cerr << "Unable to open file" << endl;
		return 1;
	}

	//Find file size from file path
	num_size = filesize(file_path);

	//Convert file size from number to string
	snprintf(file_size, sizeof(file_size), "%zu", num_size);

	//Find file name from file path
	strncat(file_name, basename(file_path), sizeof(file_name) - 1);

	cout << file_name << " is " << file_size << " bytes" << endl;

	//Send file name and file size to server
	socket_write(my_socket, file_name, sizeof(file_name));
	socket_write(my_socket, file_size, sizeof(file_size));

	//Send actual data
	while ((current_data = fread(my_buffer, 1, my_buffer_size, file)) > 0){

		if (write(my_socket, my_buffer, current_data) == 0){
			cerr << endl << "Write error" << endl;
			exit(1);
		}
		written_data += current_data;
		
		cout << "Sending data... " << written_data << "/" << file_size << "\r" << flush;
	}

	if (written_data == num_size){
		cout << endl << "Data sent" << endl;
	} else {
		cerr << endl << "Failed to send" << endl;
	}

	fclose(file);
	close(my_socket);

	return 0;
}

int receive_file(int &my_socket, char my_buffer[], size_t my_buffer_size){

	FILE * file;
	char file_name[256], save_name[256+10] = {0}, file_size[16];
	size_t current_data, read_data = 0, num_size;

	//Receive file name and file size from server
	socket_read(my_socket, file_name, sizeof(file_name));
	socket_read(my_socket, file_size, sizeof(file_size));

	//Convert file size from string to number
	sscanf(file_size, "%zu", &num_size);

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
	while (read_data < num_size){

		if ((current_data = read(my_socket, my_buffer, my_buffer_size)) == 0){
			cerr << endl << "Read error" << endl;
			exit(1);
		}
		read_data += current_data;

		fwrite(my_buffer, 1, current_data, file);
			
		cout << "Receiving data... " << read_data << "/" << file_size << "\r" << flush;
	}

	if (read_data == num_size){
		cout << endl << "Data received" << endl;
	} else {
		cerr << endl << "Failed to receive" << endl;
	}

	fclose(file);
	close(my_socket);

	return 0;
}
