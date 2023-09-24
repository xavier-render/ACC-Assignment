// your port numbers : [50150 - 50154]
// to find IP address: `ipconfig getifaddr en0`
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/select.h>

#include    "acc.h"
#include    "functions.h"

int client_count = 0;
int main() {
    int server_socket;
    struct sockaddr_in server_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    // Create socket [server_socket]
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);
    
    /**
    *
    * @brief This loop continuously accepts incoming client connections and delegates each to a new thread for communication.
    *
    */

    while (1) {
        // allocate memory for storing the client socket file descriptor 
        int* client_socket = malloc(sizeof(int));
        
        // accept an incoming connection from a client, store the client socket descriptor in client_socket
        *client_socket = accept_new_connection(server_socket, (struct sockaddr *)&server_addr, &client_addr_len);

        // check if the accept operation failed 
        if (*client_socket == -1) {
            perror("accept");
            free(client_socket);
            continue;
        }

        // print a message indicating that a new connectiono was accepted, including the `client's IP address` and `port` 
        printf("Accepted a new connection from IP address: %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

        // create a new thread to handle communication with the client 
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket) != 0) {
            perror("pthread_create");   // print an error message if thread creation failed
            close(*client_socket);     // close the client socket  
            free(client_socket);      // free the memory allocated for the client_socket 
        }
    }

    close(server_socket);
    return 0;
}