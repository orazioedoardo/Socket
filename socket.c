#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SEND 1
#define RECEIVE 0
#define SERVER 1
#define CLIENT 0

/*-------------------------------------------------------------*/

void show_help(char *);
int is_valid_ip(char *);
int file_exists(char *);
off_t stat_size(char *);
void socket_read(int, void *, size_t);
void socket_write(int, void *, size_t);
char * change_name(char *, uint16_t);
void send_file(int, char *);
void receive_file(int);
int start_server(int);
int accept_client(int);
int connect_to_server(char *, int);

/*-------------------------------------------------------------*/

int main(int argc, char * argv[]){

	char * host = NULL;
	char * file = NULL;
	int port = 0;

	uint8_t option = CLIENT;
	uint8_t mode = SEND;

	if (argc == 1 || (argc == 2 && strcmp(argv[1], "-h") == 0)){
		show_help(argv[0]);
		return 0;
	}

	for (unsigned i = 0; i < argc; i++){

		if (strcmp(argv[i], "-l") == 0 && (i+2 <= argc)){

			port = atoi(argv[i+1]);
			option = SERVER;

		} else if (strcmp(argv[i], "-r") == 0){

			mode = RECEIVE;

		} else if (strcmp(argv[i], "-s") == 0 && (i+2 <= argc)){

			file = argv[i+1];

		} else if (strcmp(argv[i], "-c") == 0 && (i+3 <= argc)){

			host = argv[i+1];
			port = atoi(argv[i+2]);
		}
	}

	if (port < 1024 || port > 65535){
		fprintf(stderr, "Invalid port number (use -h to show the help)\n");
		return 1;
	}

	if (mode == SEND){
		FILE * test = fopen(file, "r");
		if (test == NULL){
			fprintf(stderr, "Unable to open file: %s (use -h to show the help)\n", strerror(errno));
			return 1;
		}
		fclose(test);
	}

	if (option == CLIENT){
		if (!is_valid_ip(host)){
			fprintf(stderr, "Invalid ip address (use -h to show the help)\n");
			return 1;
		}
	}

	int socket_fd, new_socket_fd;
	uint8_t response = mode;

	if (option == SERVER){

		socket_fd = start_server(port);
		new_socket_fd = accept_client(socket_fd);

		socket_read(new_socket_fd, &response, sizeof(response));
		socket_write(new_socket_fd, &mode, sizeof(mode));

		if (response == mode){
			fprintf(stderr, "Operation mode mismatch\n");
			return 1;
		}

		if (mode == SEND){
			send_file(new_socket_fd, file);
		} else {
			receive_file(new_socket_fd);
		}

		close(new_socket_fd);

	} else {

		socket_fd = connect_to_server(host, port);

		socket_write(socket_fd, &mode, sizeof(mode));
		socket_read(socket_fd, &response, sizeof(response));

		if (response == mode){
			fprintf(stderr, "Operation mode mismatch\n");
			return 1;
		}

		if (mode == SEND){
			send_file(socket_fd, file);
		} else {
			receive_file(socket_fd);
		}
	}

	close(socket_fd);
}

/*-------------------------------------------------------------*/

void show_help(char * name){
	printf("Usage: %s [-l port | -c host port] [-s file | -r]\n\n", name);
	printf("Examples:\n");
	printf("  %s -l 1234 -s image.jpg            Listen on port 1234 and send image.jpg\n", name);
	printf("  %s -l 1234 -r                      Listen on port 1234 and receive a file\n", name);
	printf("  %s -c 127.0.0.1 1234 -s image.jpg  Connect to 127.0.0.1 on port 1234 and send image.jpg\n", name);
	printf("  %s -c 127.0.0.1 1234 -r            Connect to 127.0.0.1 on port 1234 and receive a file\n\n", name);
	printf("Server's firewall needs to accept incoming connections on the provided port, which should be >= 1024 and <= 65535\n");
	printf("If \"file\" is already present on destination, a random string will be prepended to its name before saving\n");
}

int is_valid_ip(char * ip){

	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ip, &sa.sin_addr);
	return (result != 0);
}

int file_exists(char * file_path){

	struct stat st;
	return (stat(file_path, &st) == 0);
}

off_t stat_size(char * file_path){

	struct stat st;
	stat(file_path, &st);
	return st.st_size;
}

void socket_read(int socket_fd, void * data, size_t data_size){

	if (read(socket_fd, data, data_size) < data_size){
		fprintf(stderr, "Read error\n");
		exit(1);
	}
}

void socket_write(int socket_fd, void * data, size_t data_size){

	if (write(socket_fd, data, data_size) < data_size){
		fprintf(stderr, "Write error\n");
		exit(1);
	}
}

char * change_name(char * file_name, uint16_t name_size){

	srand(time(NULL));
	char * charset = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char * new_name = malloc(11 + name_size);
	if (new_name == NULL){
		fprintf(stderr, "Allocation error\n");
		exit(1);
	}

	for (unsigned i = 0; i < 10; i++){
		new_name[i] = charset[rand()%62];
	}
	new_name[10] = '_';
	for (unsigned i = 0; i < name_size; i++){
		new_name[i+11] = file_name[i];
	}

	return new_name;
}

void send_file(int socket_fd, char * file_path){

	char buffer[1024];

	uint16_t file_data = 0;
	uint16_t socket_data = 0;
	uint64_t sent_data = 0;

	FILE * file = fopen(file_path, "r");
	if (file == NULL){
		fprintf(stderr, "Unable to open file: %s\n", strerror(errno));
		exit(1);
	}

	char * file_name = basename(file_path);
	uint64_t file_size = stat_size(file_path);
	uint16_t name_size = strlen(file_name)+1;

	printf("%s is %"PRIu64" bytes\n", file_name, file_size);

	socket_write(socket_fd, &name_size, sizeof(name_size));
	socket_write(socket_fd, file_name, name_size);
	socket_write(socket_fd, &file_size, sizeof(file_size));

	while (sent_data < file_size){

		file_data = fread(buffer, 1, sizeof(buffer), file);
		socket_data = write(socket_fd, buffer, file_data);

		if (file_data != socket_data){
			fprintf(stderr, "\nSize mismatch\n");
			exit(1);
		}
		sent_data += socket_data;

		printf("Sending data... %"PRIu64" / %"PRIu64"\r", sent_data, file_size);
		fflush(stdout);
	}

	if (sent_data == file_size){
		printf("\nData sent\n");
	} else {
		fprintf(stderr, "\nFailed to send\n");
		exit(1);
	}

	fclose(file);
}

void receive_file(int socket_fd){

	char buffer[1024];

	uint16_t socket_data = 0;
	uint16_t file_data = 0;
	uint64_t received_data = 0;

	char * file_name = NULL;
	uint64_t file_size = 0;
	uint16_t name_size = 0;

	socket_read(socket_fd, &name_size, sizeof(name_size));
	if (name_size > 245){
		fprintf(stderr, "File name too long\n");
		exit(1);
	}

	file_name = malloc(name_size);
	if (file_name == NULL){
		fprintf(stderr, "Allocation error\n");
		exit(1);
	}

	socket_read(socket_fd, file_name, name_size);
	if (file_name[name_size-1] != 0 || strlen(file_name)+1 != name_size){
		fprintf(stderr, "Invalid file name\n");
		exit(1);
	}

	socket_read(socket_fd, &file_size, sizeof(file_size));
	printf("%s is %"PRIu64" bytes\n", file_name, file_size);

	char * new_name = NULL;
	if (file_exists(file_name)){
		new_name = change_name(file_name, name_size);
		free(file_name);
		file_name = NULL;
	} else {
		new_name = file_name;
	}

	FILE * file = fopen(new_name, "w");
	if (file == NULL){
		fprintf(stderr, "Unable to open file: %s\n", strerror(errno));
		exit(1);
	}

	while (received_data < file_size){

		socket_data = read(socket_fd, buffer, sizeof(buffer));
		file_data = fwrite(buffer, 1, socket_data, file);

		if (socket_data != file_data){
			fprintf(stderr, "\nSize mismatch\n");
			exit(1);
		}
		received_data += file_data;

		printf("Receiving data... %"PRIu64" / %"PRIu64"\r", received_data, file_size);
		fflush(stdout);
	}

	if (received_data == file_size){
		printf("\nData received\n");
	} else {
		fprintf(stderr, "\nFailed to receive\n");
		exit(1);
	}

	free(file_name);
	fclose(file);
}

int start_server(int port){

	struct sockaddr_in server_addr;
	socklen_t server_size = sizeof(server_addr);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0){
		fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Successfully created socket\n");
	}

	if (bind(socket_fd, (struct sockaddr *) &server_addr, server_size) < 0){
		fprintf(stderr, "Binding failed: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Binding successful\n");
	}

	if (listen(socket_fd, 1) < 0){
		fprintf(stderr, "Failed to listen: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Listening...\n");
	}

	return socket_fd;
}

int accept_client(int socket_fd){

	struct sockaddr_in client_addr;
	socklen_t client_size = sizeof(client_addr);

	int new_socket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_size);

	if (new_socket_fd < 0){
		fprintf(stderr, "Connection failed: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Client %s connected\n", inet_ntoa(client_addr.sin_addr));
	}

	return new_socket_fd;
}

int connect_to_server(char * host, int port){

	struct sockaddr_in server_addr;
	socklen_t server_size = sizeof(server_addr);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(port);

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0){
		fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Successfully created socket\n");
	}

	if (connect(socket_fd, (struct sockaddr *) &server_addr, server_size) < 0){
		fprintf(stderr, "Connection failed: %s\n", strerror(errno));
		exit(1);
	} else {
		printf("Connection established\n");
	}

	return socket_fd;
}
