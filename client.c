#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>


#define CLIENT_PORT_U 50151   // Port for passive socket (This passive socket is created to listen for incoming connections from the server.)
#define LOG_FILE_PATH "client_log.txt" // Define the log file path

void log_command_result(const char* command, const char* result) {
    // Open the log file in append mode
    FILE* log_file = fopen(LOG_FILE_PATH, "a");

    if (log_file == NULL) {
        perror("fopen");
        return;  // If unable to open the log file, return without logging
    }

    // Get the current time as a string (you can use other timestamp formats)
    time_t current_time;
    struct tm* time_info;
    char timestamp[80];
    
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", time_info);

    // Log the command and result along with a timestamp
    fprintf(log_file, "%s Command: %s\n", timestamp, command);
    fprintf(log_file, "%s Result: %s\n\n", timestamp, result);

    // Close the log file
    fclose(log_file);
}

int main(int argc, char * argv[]) {
    int server_socket;
    if(argc < 3){
        printf("please use ./client server_ip server_port to run this \n e.g 127.0.0.1 50150\n");
        return 0;
    }

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // create a socket for communication with the server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;                      // IPV4
    server_addr.sin_port = htons(atoi(argv[2]));          //  using `htons()` to convert and set the server port
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);    //   using `inet_addr` to convert and set IP

    // Connect to the server
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Create a passive socket for receiving responses from the server
    int passive_socket;
    struct sockaddr_in passive_addr;

    passive_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (passive_socket == -1) {
        perror("socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    passive_addr.sin_family = AF_INET;
    passive_addr.sin_port = htons(CLIENT_PORT_U);   // set the passive socket's port
    passive_addr.sin_addr.s_addr = INADDR_ANY;

    // bind the passive socket to a port and address
    if (bind(passive_socket, (struct sockaddr*)&passive_addr, sizeof(passive_addr)) == -1) {
        perror("bind");
        close(server_socket);
        close(passive_socket);
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections on the passive socket
    if (listen(passive_socket, 1) == -1) {
        perror("listen");
        close(server_socket);
        close(passive_socket);
        exit(EXIT_FAILURE);
    }

    // Send the port U information to the server via request-line
    char port_u_message[32];
    snprintf(port_u_message, sizeof(port_u_message), "%d", CLIENT_PORT_U);
    send(server_socket, port_u_message, strlen(port_u_message), 0);

    // accept the response line for communication with the server
    int response_line = accept(passive_socket, (struct sockaddr*)&passive_addr, &addr_len);
    // Command Processing Loop

    // command processing loop 
    while (1) {
        printf("\nClient>");
        fflush(stdout);
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(response_line, &read_set);

        // use select to wait for data on the passive socket with a timeout
        if(select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0){
            break;
        }

        char user_command[256];
        for(int i = 0; i < FD_SETSIZE; i++){
            if(FD_ISSET(i, &read_set)){
                if(i == STDIN_FILENO){
                    fgets(user_command, sizeof(user_command), stdin);

                    // send user_command to the server via request-line
                    send(server_socket, user_command, strlen(user_command), 0);
                } 
                else if(i == response_line){
                    break;
                }
            }
        }

        // Receive and display the results (or error messages) from the server
        char server_response[1024];
        ssize_t bytes_received = recv(response_line, server_response, sizeof(server_response), 0);

        if (bytes_received <= 0) {
            perror("recv");
            break;
        }

        server_response[bytes_received] = '\0';
        printf("Server Response: %s", server_response);

        // log the command and its result 
        log_command_result(user_command, server_response);

        if(strncmp(user_command, "QUIT", 4) == 0){
            break;
        }
        if(strncmp(server_response, "\nTimeout", 7) == 0){
            break;
        }
    }

    // Handle the 'QUIT' command and clean up resources
    close(server_socket);
    close(passive_socket);

    return 0;
}
