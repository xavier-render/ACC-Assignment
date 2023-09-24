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

#include    "acc.h"     // include necessary headers

int max_client = 10;
int max_time = 120;
int current_clients = 0;

// initialise a mutex for thread safety
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_client_count_update(int value){
    pthread_mutex_lock(&mutex);         // lock the mutex
    current_clients += value;          // update the current client count safely
    pthread_mutex_unlock(&mutex);     // unlock the mutex
}

int accept_new_connection(int server_socket, struct sockaddr * server_addr, socklen_t * client_addr_len){
    pthread_mutex_lock(&mutex);             // lock mutex
    if(current_clients >= max_client)
        return -1;                         // if the maximum clients limits is reached, return -1
    pthread_mutex_unlock(&mutex);         // unlock the mutex

    int cd;
    cd = accept(server_socket, server_addr, client_addr_len);   // accept a new client connection
    if(cd > 0){
        safe_client_count_update(1);                           // increment the current client count safely 
    }
    return cd;
}

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg);

    // Receive the client's reply port U
    char buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        perror("recv: FIRST ONE");
        close(client_socket);
        safe_client_count_update(-1);
        return NULL;;
    }

    if (bytes_received >= sizeof(buffer)) {
        fprintf(stderr, "Error: Received data does not fit into buffer.\n");
        close(client_socket);
        safe_client_count_update(-1);
        return NULL;;
    }
    
    buffer[bytes_received] = '\0';
    int client_reply_port = atoi(buffer);       // convert the received port to an integer

    // Create a reply-line socket and connect to the client's port U
    int reply_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (reply_socket == -1) {
        perror("socket");
        close(client_socket);
        safe_client_count_update(-1);
        return NULL;;
    }

    // Declares variable called : `client_reply_addr` of type `struct sockaddr_in` which hold the socket address info 
    struct sockaddr_in client_reply_addr;
    bzero(&client_reply_addr, sizeof(client_reply_addr));
    client_reply_addr.sin_family = AF_INET;
    client_reply_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Loopback address
    client_reply_addr.sin_port = htons(client_reply_port);

    // this line attemps to establish connection to the client's reply port.
    if (connect(reply_socket, (struct sockaddr*)&client_reply_addr, sizeof(client_reply_addr)) == -1) {
        perror("connect");
        close(client_socket);
        close(reply_socket);
        safe_client_count_update(-1);
        return NULL;;
    }

    // set up the timeval strucutre for the timeout
    struct timeval tv;
    tv.tv_sec = max_time; // Maximum time to wait in seconds
    tv.tv_usec = 0;      // Microseconds
    
    // This part of the code is the `request-line` 
    // where the server receives commands / requests from the client in a form of text and handles it.
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);

        // Use select to wait for data on the client socket with a timeout
        int ready = select(client_socket + 1, &read_fds, NULL, NULL, &tv);
        
        if (ready == -1) {
            perror("select");
            break;
        } else if (ready == 0) {
            // Timeout occurred, send a 'goodbye' message and close both sockets
            char timeout_message[] = "\nTimeout occurred. Goodbye!\n";
            send(reply_socket, timeout_message, strlen(timeout_message), 0);
            close(client_socket);
            close(reply_socket);
            safe_client_count_update(-1);
            return NULL;;
        }
        
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if( bytes_received == 0) {
            printf("Client closed the connection. \n");
            close(client_socket);
            close(reply_socket);
            safe_client_count_update(-1);
            return NULL;;
        }
        
        if (bytes_received <= 0) {
            perror("recv: SECOND ONE");
            break;
        }

        buffer[bytes_received] = '\0';

        // Process the received command
        char response[1024];
        
        // Implement server-specific functions (add, mul, divide, mod)
        if (strncmp(buffer, "ADD", 3) == 0) {
            // Process ADD command
            int x, y;
            if (sscanf(buffer, "ADD(%d,%d)", &x, &y) == 2) {
                int result = add(x, y);
                sprintf(response, "Result: %d\n", result);    
                
            } else {
                strcpy(response, "Error: Invalid ADD command format\n");
            }
        } 
        
        else if (strncmp(buffer, "MUL", 3) == 0) {
            // Process MUL command
            int x, y;
            if (sscanf(buffer, "MUL(%d,%d)", &x, &y) == 2) {
                int result = mul(x, y);
                sprintf(response, "Result: %d\n", result);
            } else {
                strcpy(response, "Error: Invalid MUL command format\n");
            }
        } 
        
        else if (strncmp(buffer, "DIV", 3) == 0) {
            // Process DIV command
            int x, y;
            if (sscanf(buffer, "DIV(%d,%d)", &x, &y) == 2) {
                if (y != 0) {
                    int result = divide(x, y);
                    sprintf(response, "Result: %d\n", result);
                } else {
                    strcpy(response, "Error: Division by zero\n");
                }
            } else {
                strcpy(response, "Error: Invalid DIV command format\n");
            }
        } 
        
        else if (strncmp(buffer, "MOD", 3) == 0) {
            // Process MOD command
            int x, y;
            if (sscanf(buffer, "MOD(%d,%d)", &x, &y) == 2) {
                if (y != 0) {
                    int result = mod(x, y);
                    sprintf(response, "Result: %d\n", result);
                } else {
                    strcpy(response, "Error: Modulo by zero\n");
                }
            } else {
                strcpy(response, "Error: Invalid MOD command format\n");
            }
        } 
        
        else if (strncmp(buffer, "INFO", 4) == 0) {
            // Send info about available commands
            send_info(reply_socket);
            continue;  // continue listening for new commands 
        }

        else if (strncmp(buffer, "QUIT", 4) == 0) {
            //  Send a 'goodbye' message to the client
            char goodbye_message[] = "Thank you for using SFC. Goodbye!\n";
            send(reply_socket, goodbye_message, strlen(goodbye_message), 0);
        
            // Close both sockets when done
            close(client_socket);
            close(reply_socket);
            safe_client_count_update(-1);
            return NULL;;
        }
        else {
            strcpy(response, "Error: Invalid Command\n");
        }

        // Send the response to the client
        // the 'send' function is part of the socket programming API and can be found in `<sys/socket>` header
        send(reply_socket, response, strlen(response), 0);
    }
    safe_client_count_update(-1);
    return NULL;;
}

// Function to send info about available commands to the client
void send_info(int client_socket) {
    FILE* info_file = fopen("info.txt", "r");
    if (info_file == NULL) {
        perror("info file");
        return;
    }

    char info_buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(info_buffer, 1, sizeof(info_buffer), info_file)) > 0) {
        send(client_socket, info_buffer, bytes_read, 0);
    }

    fclose(info_file);
}

// log add result into a file 
void log_add(int x, int y, int result) {

    // open the log file in `append` mode to add log entries at the end.
    FILE* log_file = fopen("log_file.txt", "a");

    if(log_file == NULL) {
        perror("fopen");
    }

    if (log_file != NULL) {
        // log the add operation among with its operands and result 
        fprintf(log_file, "ADD(%d,%d); the result is: %d\n", x, y, result);

        // close the log file.
        fclose(log_file);
    } else {
        // Handle error opening the log file
        printf("Error: Could not open log_file.txt for writing.\n");
    }
}

// log mul result into a file 
void log_mul(int x, int y, int result) {

    // open the log file in `append` mode to add log entries at the end.
    FILE* log_file = fopen("log_file.txt", "a");

    if(log_file == NULL) {
        perror("fopen");
    }

    if (log_file != NULL) {
        // log the multiple operation among with its operands and result 
        fprintf(log_file, "MULTPIPLE(%d,%d); the result is: %d\n", x, y, result);

        // close the log file.
        fclose(log_file);
    } else {
        // Handle error opening the log file
        printf("Error: Could not open log_file.txt for writing.\n");
    }
}

// log div result into a file 
void log_divide(int x, int y, int result) {

    // open the log file in `append` mode to add log entries at the end.
    FILE* log_file = fopen("log_file.txt", "a");

    if(log_file == NULL) {
        perror("fopen");
    }
    if (log_file != NULL) {
        // log the divide operation among with its operands and result 
        fprintf(log_file, "DIVIDE(%d,%d); the result is: %d\n", x, y, result);

        // close the log file.
        fclose(log_file);
    } else {
        // Handle error opening the log file
        printf("Error: Could not open log_file.txt for writing.\n");
    }
}

// log mod result into a file 
void log_mod(int x, int y, int result) {
    // Log the operation to a file (log_file)
    FILE* log_file = fopen("log_file.txt", "a");

    if(log_file == NULL) {
        perror("fopen");
    }

    if (log_file != NULL) {
        // log the mod operation among with its operands and result 
        fprintf(log_file, "MOD(%d,%d); the result is: %d\n", x, y, result);
        
        // close the log file.
        fclose(log_file);
    } else {
        // Handle error opening the log file
        printf("Error: Could not open log_file.txt for writing.\n");
    }
}

// Function to perform the ADD operation
int add(int x, int y) {
    int result = x + y;
    log_add(x, y, result);
    return result; // Return the result
}

// Function to perform the MULTIPLICATION operation
int mul(int x, int y){
    int result = x * y;
    log_mul(x, y, result);
    return result;
}

// Function to perform the DIVIDE operation
int divide(int x, int y) {
    int result = x / y;
    log_mul(x, y, result); // Log the result using log_mul
    return result;
}

// Function to perform the MOD operation
int mod(int x, int y) {
    int result = x % y;
    log_mod(x, y, result); // Log the result using log_mul
    return result;
}