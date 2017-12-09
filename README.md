# Socket

This is the code I've created to practice with C sockets, you can use it to send and receive files over the network. 
It can act both as a server or client, once the connection is estabilished, the file exchange is bidirectional.

###Example

```
server$ ./socket -l 4444 -r
Successfully created socket
Binding successful
Listening...
Client 127.0.0.1 connected
demo is 52428800 bytes
Receiving data... 52428800 / 52428800
Data received
```

```
client$ ./socket -c 127.0.0.1 4444 -s demo 
Successfully created socket
Connection established
demo is 52428800 bytes
Sending data... 52428800 / 52428800
Data sent
```

###Additional information

Tested on OS X and Linux.
To check whether the server is actually listening or not run ```lsof -i -P | grep socket``` on Mac or ```sudo netstat -antp | grep socket``` on Linux.

The connection is unencrypted so avoid using it over WAN, also there is no integrity check. Improvements that could be made are the ability to deal with multiple clients, additional checks on argv, file I/O and error management.
