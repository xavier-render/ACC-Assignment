#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/select.h>

#include  "functions.h"

#define PORT 50150
#define BACKLOG 5
#define MAX_CLIENTS 10
#define MAX_TIME 120