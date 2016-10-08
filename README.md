# Socket

This is the code I've created to practice with C sockets, you can use it to send and receive files over the network. 
It can act both as a server or client.

The server listens on a port (the firewall should accept incoming connections from that port).
The client connects to the server via specified ip address and port number, then it asks whether to send or receive a file.

At this point, if sending a file, you write the file path, then the filename and its size are determined and sent to the other peer.
File is sent (and received) in 1024 bytes chunks. The other peer retrieves file information and saves the file as received_filename.

###How to build and use

Clone this repository, cd into the folder and run: ```g++ -o socket *.cpp -std=c++11```

Start the server first with ```./socket -l port```, then connect the client with ```./socket -c ip_address port```.

###Demo usage

```
server$ ./socket -l 4444
Successfully created socket
Binding successful
Listening...
Client 127.0.0.1 connected
demo.dat is 10485760 bytes
Receiving data... 10485760/10485760
Data received
```

```
client$ ./socket -c 127.0.0.1 4444
Successfully created socket
Connection established
Send or receive file? [1/2] 1
Choose a file: demo.dat
demo.dat is 10485760 bytes
Sending data... 10485760/10485760
Data sent
```

###Additional information

Tested on OS X and Linux.
To check whether the server is actually listening or not run ```lsof -i -P | grep socket``` on Mac or ```sudo netstat -antp | grep socket``` on Linux.

The connection is unencrypted so avoid using it over WAN, also there is no integrity check. Improvements that could be made are the ability to deal with multiple clients, addition of checks on argv, filepath parsing, file I/O and error syncronizations between client and server.
