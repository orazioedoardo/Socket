//Socket functions
void connect_to_server(int &my_socket, sockaddr_in &my_server);

void create_socket(int &my_listener);
void bind_port(int &my_listener, sockaddr_in &my_server);

void listen_for_client(int &my_listener);
void accept_client(int &my_socket, int &my_listener, sockaddr_in &my_server, sockaddr_in &my_client);

//File exchange
void generic_read(int &my_socket, char data[], size_t data_size);
void generic_write(int &my_socket, char data[], size_t data_size);

size_t get_file_size(char file_path[]);
void get_file_name(char file_path[], char file_name[]);
void get_save_name(char file_name[], char save_name[]);

int send_file(int &my_socket, char my_buffer[], size_t my_buffer_size);
int receive_file(int &my_socket, char my_buffer[], size_t my_buffer_size);