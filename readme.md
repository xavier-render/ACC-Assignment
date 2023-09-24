ACC Assignment S2 2023

Project

Simple Function Call
The objective of this network programming assignment is to develop two complementary programs,
a client and a server, which implement a simple application program, called Simple Function Call (SFC).

To compile

type `make` in the terminal

To remove objects created

type `make clean` in the terminal

Run the program

type `make` in the terminal, to compile both `client` and `server` programs
type `./server` in termainal and open another terminal and type `./client 127.0.0.1 50150 `

After, sucessful connection you can then use these commands

Available commands:
  ADD(x,y) - Add two numbers.
  MUL(x,y) - Multiply two numbers.
  DIV(x,y) - Divide two numbers.
  MOD(x,y) - Calculate the modulo of two numbers.
  INFO     - Get a list of available commands.
  QUIT     - Quit the program and close the connection.
