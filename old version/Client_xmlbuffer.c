#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define PORT 24919

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
        xmlChar *name = xmlGetProp(root, (const xmlChar *)"name");
        xmlChar *version = xmlGetProp(root, (const xmlChar *)"version");

        printf("Laser Server Name: %s\n", name);
        printf("Laser Server Version: %s\n", version);

        // Free the properties
        xmlFree(name);
        xmlFree(version);
    } else {
        printf("Unexpected root element: %s\n", root->name);
    }

    // Free the document
    xmlFreeDoc(doc);

    // Cleanup function for the XML library
    xmlCleanupParser();
}

int main(int argc, char const *argv[]) {
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char messageBuffer[4096] = {0}; // Larger buffer to accumulate data
    size_t messageLen = 0;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading

        // Read message from the server
        valread = read(client_fd, buffer, sizeof(buffer) - 1); // subtract 1 for the null terminator at the end
        if (valread <= 0) {
            printf("Server disconnected or error occurred\n");
            break;
        }

        buffer[valread] = '\0'; // Ensure null-terminated string

        // Accumulate data into the message buffer
        strncat(messageBuffer, buffer, sizeof(messageBuffer) - strlen(messageBuffer) - 1);
        messageLen += valread;

        // Check if we have received a complete message
        if (strstr(messageBuffer, "</ulmsserver>")) {
            // Print the raw XML message
            printf("Received XML message:\n%s\n", messageBuffer);

            // Parse the XML message
            parseXMLMessage(messageBuffer);

            // Clear the message buffer for the next message
            memset(messageBuffer, 0, sizeof(messageBuffer));
            messageLen = 0;
        }
    }

    close(client_fd);
    return 0;
}
