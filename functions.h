#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
int accept_new_connection(int server_socket, struct sockaddr * server_addr, socklen_t * client_addr_len);
void* handle_client(void* arg);
void send_info(int client_socket);

int add(int x, int y);
int mul(int x, int y);
int divide(int x, int y);
int mod(int x, int y);

void log_add(int x, int y, int result);
void log_mul(int x, int y, int result);
void log_divide(int x, int y, int result);
void log_mod(int x, int y, int result);