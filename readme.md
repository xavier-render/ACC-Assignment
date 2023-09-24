ACC Assignment S2 2023

Project

The purpose of the program is to facilitate communication between a server and multiple clients using two parallel TCP connections (request-line and reply-line), allowing clients to request and execute server functions while logging each request and response in a .txt file 

To compile

type `make` in the terminal

To remove objects created

type `make clean` in the terminal

Run the program

type `make` in the terminal, to compile both `client` and `server` programs
type `./server` in terminal and open another terminal and type `./client 127.0.0.1 50150 `

After, sucessful connection you can then use these commands

Available commands:
  ADD(x,y) - Add two numbers.
  MUL(x,y) - Multiply two numbers.
  DIV(x,y) - Divide two numbers.
  MOD(x,y) - Calculate the modulo of two numbers.
  INFO     - Get a list of available commands.
  QUIT     - Quit the program and close the connection.
