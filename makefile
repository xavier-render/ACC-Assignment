CC = gcc
OBJ = server.o functions.o
EXEC = server
CFLAGS = -Wall -pthread

#--------PROGRAM-EXECUTEABLES-----------------------------------------
all: calculator client

client: client.c
	$(CC) client.c -o client

calculator: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(EXEC)

#--------OBJECTS------------------------------------------------------
server.o: server.c acc.h
	$(CC) $(CFLAGS) server.c -c

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) functions.c -c

#--------UTILS--------------------------------------------------------
clean:
	rm -f $(OBJ) $(EXEC) client


