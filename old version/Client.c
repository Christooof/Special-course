// Client side C program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
//client will connect to port 8080
#define PORT 8080
 
// ./program arg1 arg2 arg3
// ./program is the name of the executable.
// arg1, arg2, and arg3 are command-line arguments.
// they are parsed for compatibility and adherence to conventions 
int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    //client_fd: file descriptor, unique identifier used to represent an open file, socket blah
    //with that you can refer to that resource
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    //sockaddr: address of server the client wants to connect (IP address port number)
    //connect(): initiates a connection between client and server
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    //send the message hello
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    //it reads the received data into the buffer
    valread = read(client_fd, buffer,
                   1024 - 1); // subtract 1 for the null
                              // terminator at the end
    printf("%s\n", buffer);
 
    // closing the connected socket
    close(client_fd);
    return 0;
}
