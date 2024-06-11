#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define MAX_BUFFER_SIZE 1024

// Structure to represent the laser server connection
typedef struct {
    int port;
    char host[256];
    char name[256];
    int status;
    int config;
    int sockfd;
} componentservertype;

// Global variable for laser server connection
componentservertype lmssrv;

// Function prototypes
void receiveFromLaserServer(int sockfd);
void parseXMLMessage(const char *message);

int main() {
    // Initialize lmssrv structure
    lmssrv.port = 24919;
    strcpy(lmssrv.host, "127.0.0.1");
    strcpy(lmssrv.name, "laserserver");
    lmssrv.status = 1;
    lmssrv.config = 1;

    // Create a socket for the laser server
    if (lmssrv.config) {
        int errno = 0, len;
        lmssrv.sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lmssrv.sockfd < 0) {
            perror(strerror(errno));
            fprintf(stderr, " Can not make socket\n");
            exit(errno);
        }

        // Initialize the server address structure
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(lmssrv.port);
        inet_pton(AF_INET, lmssrv.host, &server_addr.sin_addr);

        // Connect to the laser server
        if (connect(lmssrv.sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error connecting to laser server");
            exit(EXIT_FAILURE);
        }

        // Print a message upon successful connection
        printf("Connected to laser server\n");

        // Initialize an XML parser for laser data
        xmlDocPtr xmllaser = xmlNewDoc(BAD_CAST "1.0");

        // Send a command to the laser server
        if (lmssrv.status && lmssrv.config) {
            char buf[256];
            len=sprintf(buf,"scanpush cmd='zoneobst'\n");
            send(lmssrv.sockfd, buf, len, 0);
        }


        // Main loop to continuously receive and process data from the laser server
        while (1) {
            // Check if the laser server socket is valid (connected)
            if (lmssrv.sockfd >= 0) {
                receiveFromLaserServer(lmssrv.sockfd);
            }

            // Add any other processing or control logic here

            // Sleep for a short period to avoid busy-waiting
            usleep(100000); // Sleep for 100 milliseconds
        }

        // Close the socket connection before exiting
        close(lmssrv.sockfd);
    }

    return 0;
}



// Function to receive and process data from the laser server
void receiveFromLaserServer(int sockfd) {
    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;

    // Receive data from the laser server
    bytesReceived = recv(sockfd, buffer, MAX_BUFFER_SIZE - 1, 0);
    if (bytesReceived < 0) {
        perror("Error receiving data from laser server");
        return;
    }

    // Null-terminate the received data
    buffer[bytesReceived] = '\0';

    // Print the received data
    printf("Received data from laser server: %s\n", buffer);

    // Process the received data further if needed
    // For example, parse XML data, extract information, etc.
}


void parseXMLMessage(const char *message) {
    // Initialize the library and check potential ABI mismatches
    LIBXML_TEST_VERSION

    // Parse the XML message
    xmlDocPtr doc = xmlReadMemory(message, strlen(message), "noname.xml", NULL, 0);
    if (doc == NULL) {
        printf("Failed to parse XML\n");
        return;
    }

    // Get the root element
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL) {
        printf("Empty XML document\n");
        xmlFreeDoc(doc);
        return;
    }

    // Process the root element
    if (!xmlStrcmp(root->name, (const xmlChar *)"laserServer")) {
        // If the XML message is from the laser server, handle it accordingly
        // You can add code here to process laser server data
        // For example, extracting obstacle information
    } else {
        printf("Unexpected root element: %s\n", root->name);
    }

    // Free the document
    xmlFreeDoc(doc);

    // Cleanup function for the XML library
    xmlCleanupParser();
}





/*
int main() {
    // Initialize lmssrv structure
    lmssrv.port = 24919;
    strcpy(lmssrv.host, "127.0.0.1");
    strcpy(lmssrv.name, "laserserver");
    lmssrv.status = 1;
    lmssrv.config = 1;

    // Create a socket for the laser server
    if (lmssrv.config) {
        int errno = 0, len;
        lmssrv.sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lmssrv.sockfd < 0) {
            perror(strerror(errno));
            fprintf(stderr, " Can not make socket\n");
            exit(errno);
        }

        // Initialize the server address structure
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(lmssrv.port);
        inet_pton(AF_INET, lmssrv.host, &server_addr.sin_addr);

        // Connect to the laser server
        if (connect(lmssrv.sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error connecting to laser server");
            exit(EXIT_FAILURE);
        }

        // Print a message upon successful connection
        printf("Connected to laser server\n");

        // Main loop to continuously receive and process data from the laser server
        while (1) {
            // Check if the laser server socket is valid and connected
            if (lmssrv.status && lmssrv.connected) {
                receiveFromLaserServer(lmssrv.sockfd);
            }

            // Add any other processing or control logic here

            // Sleep for a short period to avoid busy-waiting
            usleep(100000); // Sleep for 100 milliseconds
        }

        // Close the socket connection before exiting
        close(lmssrv.sockfd);
    }

    return 0;
}



#define MAX_BUFFER_SIZE 1024
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

componentservertype lmssrv;

// Initialize lmssrv structure
lmssrv.port = 24919;
strcpy(lmssrv.host, "127.0.0.1");
strcpy(lmssrv.name, "laserserver");
lmssrv.status = 1;
lmssrv.config = 1;

// Create a socket for the laser server
if (lmssrv.config) {
    char buf[256];
    int errno = 0, len;
    lmssrv.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lmssrv.sockfd < 0) {
        perror(strerror(errno));
        fprintf(stderr, " Can not make socket\n");
        exit(errno);
    }

    // Connect to the laser server
    serverconnect(&lmssrv);

    // Initialize an XML parser for laser data
    xmllaser = xml_in_init(4096, 32);
    printf(" laserserver xml initialized \n");

    // Send a command to the laser server
    if (lmssrv.connected) {
        len = sprintf(buf, "push  t=0.2 cmd='mrcobst width=0.4'\n");
        send(lmssrv.sockfd, buf, len, 0);
    }
}*/
