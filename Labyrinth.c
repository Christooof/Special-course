#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <time.h>

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

// Global variablex to store certain laser values
double l4 = 1.0;
double l0=0.7;
double l8=0.7;

// Function prototypes
void receiveFromLaserServer(int sockfd);
void parseXMLMessage(const char*, int);

int main() {

    //timer creation
    double last_time = ((double) clock()) / CLOCKS_PER_SEC;
    double current_time = ((double) clock()) / CLOCKS_PER_SEC;

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

    //Initialize pointers for certain commands that are often used
    char* fwd_mrc = "drive @v0.3 :($l4<0.5) | ($l0>0.8) | ($l8>0.8)\n";
    char* extra_dis = "drive @v0.3 :($l4<0.4)\n";
    char* extra_dis_fwd = "fwd 0.6\n";
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


    //Initial commands that are send to the mrc to start the robot. 
    //send(client_fd_mrc, "log \"$time\" \"$l0\" \"$l4\" \"$l8\"\n", strlen("log \"$time\" \"$l0\" \"$l4\" \"$l8\"\n"), 0);
    send(client_fd_mrc, "laser \"scanpush cmd='zoneobst'\"\n", strlen("laser \"scanpush cmd='zoneobst'\"\n"), 0);
    send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
    send(client_fd_mrc, "fwd 0.1\n", strlen("fwd 0.1\n"), 0);
    send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
    printf("Program started\n");


    valread_mrc = read(client_fd_mrc, buffer_mrc,
                       1024 - 1); // subtract 1 for the null
                                 // terminator at the end
    printf("%s\n", buffer_mrc);

    // closing the connected socket
    //close(client_fd_mrc);

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

        
        // Main loop to continuously receive and process data from the laser server. Additionally, commands are send to the robot.
        while (1) {

            // Check if the laser server socket is valid (connected)
            if (lmssrv.sockfd >= 0) {
                receiveFromLaserServer(lmssrv.sockfd);
            }

            current_time = ((double) clock()) / CLOCKS_PER_SEC;
            
            //If statement to check if the robot is at a crossroad or at a dead-end street
            if (l4<0.5||l0>0.8||l8>0.8){
                
                //this if statement ensures that the commands are sent only once and not 100 times
                if(current_time - last_time >= 0.08){

                    //This is the definition of the end. To each side the robot has plenty of space. Once the goal is reached the robot does not move
                    if(l0>8 && l4>8 && l8>8){
                    send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                    }

                    //at crossing: if l4 is the biggest laser value, the robot continues to go straight
                    else if(l4>=l0&&l4>=l8){                    
                        printf("go straight:  l0:%f, l4:%f, l8:%f, Time: %f\n",l0,l4,l8,current_time);
                        //this if statement ensures that the robot crosses the crossing before it receives new commands
                        if(l4<1){
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                        }else{                            
                            send(client_fd_mrc, extra_dis_fwd, strlen(extra_dis_fwd), 0);
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                        }                       
                        send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                    }

                    //at crossing: if l0 is the biggest laser value, the robot turns left
                    else if(l0>l4&&l0>=l8){
                        printf("turn left:  l0:%f, l4:%f, l8:%f, Time: %f\n",l0,l4,l8,current_time);
                        //to insure that the robot is in the middle of the crossing to following successfully turn without touching the borders. 
                        if(l4<1){
                            send(client_fd_mrc, extra_dis, strlen(extra_dis), 0);
                        }else{
                            send(client_fd_mrc, extra_dis_fwd, strlen(extra_dis_fwd), 0);
                        }                        
                        send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                        send(client_fd_mrc, "turn 90\n", strlen("turn 90\n"), 0);

                        //if l0<1: the robot is probably at a dead end so should not go too far.
                        //Otherwise, after the turn the robot should go straight until it reaches the next crossing or dead end.
                        if(l0<1){
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                        }else{                            
                            send(client_fd_mrc, extra_dis_fwd, strlen(extra_dis_fwd), 0);
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                            send(client_fd_mrc, "fwd 0.1\n", strlen("fwd 0.1\n"), 0);
                        }                        
                        send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                    }

                    //at crossing: if l8 is the biggest laser value, the robot turns right
                    else if(l8>l0&&l8>l4){
                        printf("turn right:  l0:%f, l4:%f, l8:%f, Time: %f\n",l0,l4,l8,current_time);
                        //to insure that the robot is in the middle of the crossing to following successfully turn without touching the borders. 
                        if(l4<1){
                            send(client_fd_mrc, extra_dis, strlen(extra_dis), 0);
                        }else{
                            send(client_fd_mrc, extra_dis_fwd, strlen(extra_dis_fwd), 0);
                        } 
                        send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                        send(client_fd_mrc, "turn -90\n", strlen("turn -90\n"), 0);

                        //if l0<1: the robot is probably at a dead end so should not go too far.
                        //Otherwise, after the turn the robot should go straight until it reaches the next crossing or dead end.
                        if(l8<1){
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                        }else{                            
                            send(client_fd_mrc, extra_dis_fwd, strlen(extra_dis_fwd), 0);
                            send(client_fd_mrc, fwd_mrc, strlen(fwd_mrc), 0);
                            send(client_fd_mrc, "fwd 0.1\n", strlen("fwd 0.1\n"), 0);
                        }                       
                        send(client_fd_mrc, "stop\n", strlen("stop\n"), 0);
                    }

                    // Reset the timer
                    last_time = current_time;


                }
            }            
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
    parseXMLMessage(buffer, strlen(buffer));
}

//Function to parse the XML Message
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
  xmlChar* l0_value = xmlGetProp(root, (const xmlChar *)"l0");
  xmlChar* l8_value = xmlGetProp(root, (const xmlChar *)"l8");
  if (l4_value != NULL) {
    l0=atof(l0_value);
    xmlFree(l0_value); // Free the memory allocated for the attribute value
    l4=atof(l4_value);
    xmlFree(l4_value);
    l8=atof(l8_value);
    xmlFree(l8_value);    
  } else {
    printf("Attribute l4 not found\n");
  }
  // Free the document
  xmlFreeDoc(document);
  // Cleanup function for the XML library
  xmlCleanupParser();
}

