// Client side C program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 24919

int main(int argc, char const *argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1)
    {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading

        // Read message from the server
        valread = read(client_fd, buffer, sizeof(buffer) - 1); // subtract 1 for the null terminator at the end
        if (valread <= 0)
        {
            printf("Server disconnected or error occurred\n");
            break;
        }

        // Store the message in a variable
        char received_message[1024];
        strncpy(received_message, buffer, sizeof(received_message) - 1);
        received_message[sizeof(received_message) - 1] = '\0'; // Ensure null-terminated string

        // Print the received message
        printf("Received message: %s\n", received_message);
    }

    // Close the connected socket
    close(client_fd);
    return 0;
}

