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

#define PORTMRC 31001


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

// Global variable to store the value of l4
double l4 = 0.0;

// Function prototypes
void receiveFromLaserServer(int sockfd);
void parseXMLMessage(const char*, int);
//void parseXMLMessage(char *message);

int main() {
    // Initialize lmssrv structure
    lmssrv.port = 24919;
    strcpy(lmssrv.host, "127.0.0.1");
    strcpy(lmssrv.name, "laserserver");
    lmssrv.status = 1;
    lmssrv.config = 1;

    //Initialize mrc connection
    
    #define PORT_MRC 31001
    int status_mrc, valread_mrc, client_fd_mrc;
    struct sockaddr_in serv_addr_mrc;
    char* fwd_mrc = "drive @v 0.3 :($irdistfrontmiddle<0.4)\n";
    char buffer_mrc[1024] = {0};

    if ((client_fd_mrc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr_mrc.sin_family = AF_INET;
    serv_addr_mrc.sin_port = htons(PORT_MRC);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr_mrc.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status_mrc = connect(client_fd_mrc, (struct sockaddr*)&serv_addr_mrc,
                              sizeof(serv_addr_mrc))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
    printf("Hello message sent\n");
    send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);

    valread_mrc = read(client_fd_mrc, buffer_mrc,
                       1024 - 1); // subtract 1 for the null
                                 // terminator at the end
    printf("%s\n", buffer_mrc);

    // closing the connected socket
    close(client_fd_mrc);



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
            //usleep(100000); // Sleep for 100 milliseconds
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
    //printf("Received data from laser server: %s\n",buffer);


    //int message_len = strlen(buffer);
    parseXMLMessage(buffer, strlen(buffer));

    //parseXMLMessage(buffer_ptr);
    //printf(type(buffer));
    // Process the received data further
    
    //char xml_message[] = "<laser l0=\"1000\" l1=\"1000\" l2=\"1000\" l3=\"1000\" l4=\"2.44\" l5=\"1000\" l6=\"1000\" l7=\"1000\" l8=\"1000\" />";
    //parseXMLMessage(xml_message, strlen(xml_message));
    
}

void parseXMLMessage(const char* xml_message, int message_len) {
  // Initialize the library and check potential ABI mismatches
  LIBXML_TEST_VERSION

  // Parse the XML message
  xmlDoc *document = xmlReadMemory(xml_message, message_len, "noname.xml", NULL, 0);
  if (document == NULL) {
    fprintf(stderr, "Failed to parse XML\n");
    return;
  }

  // Get the root element node
  xmlNode *root = xmlDocGetRootElement(document);
  if (root == NULL) {
    fprintf(stderr, "Empty XML document\n");
    xmlFreeDoc(document);
    return;
  }

  // Extract the value of the attribute l4
  xmlChar* l4_value = xmlGetProp(root, (const xmlChar *)"l4");
  if (l4_value != NULL) {
    //printf("The value of l4 is: %s\n", l4_value);
    l4=atof(l4_value);
    printf("The value of l4 is: %f\n",l4);
    xmlFree(l4_value); // Free the memory allocated for the attribute value
  } else {
    printf("Attribute l4 not found\n");
  }

  // Free the document
  xmlFreeDoc(document);

  // Cleanup function for the XML library
  xmlCleanupParser();
}




/*
void parseXMLMessage(char* xml_message) {
    // Initialize the library and check potential ABI mismatches
    LIBXML_TEST_VERSION
    //printf(xml_message);
    // Parse the XML message
    xmlDoc *document = xmlReadMemory(xml_message, strlen(xml_message), "noname.xml", NULL, 0);
    if (document == NULL) {
        fprintf(stderr, "Failed to parse XML\n");
        return;
    }

    // Get the root element node
    xmlNode *root = xmlDocGetRootElement(document);
    if (root == NULL) {
        fprintf(stderr, "Empty XML document\n");
        xmlFreeDoc(document);
        return;
    }

    // Extract the value of the attribute l4
    xmlChar* l4_value = xmlGetProp(root, (const xmlChar *)"l4");
    if (l4_value != NULL) {
        printf("The value of l4 is: %s\n", l4_value);
        xmlFree(l4_value); // Free the memory allocated for the attribute value
    } else {
        printf("Attribute l4 not found\n");
    }

    // Free the document
    xmlFreeDoc(document);
    // Cleanup function for the XML library
    xmlCleanupParser();
}



void parseXMLMessage(const char *message) {
    // Ensure the XML message is well-formed
    char xmlBuffer[MAX_BUFFER_SIZE];
    snprintf(xmlBuffer, MAX_BUFFER_SIZE, "<root>%s</root>", message);

    // Initialize the library and check potential ABI mismatches
    LIBXML_TEST_VERSION

    // Parse the XML message
    xmlDocPtr doc = xmlReadMemory(xmlBuffer, strlen(xmlBuffer), "noname.xml", NULL, 0);
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
    for (xmlNodePtr cur = root->children; cur != NULL; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE && !xmlStrcmp(cur->name, (const xmlChar *)"laser")) {
            // Extract the l4 attribute
            xmlChar *l4Attr = xmlGetProp(cur, (const xmlChar *)"l4");
            if (l4Attr != NULL) {
                l4_value = atof((const char *)l4Attr);
                printf("l4 value: %f\n", l4_value);
                xmlFree(l4Attr);
            }
        }
    }

    // Free the document
    xmlFreeDoc(doc);

    // Cleanup function for the XML library
    xmlCleanupParser();
}
*/