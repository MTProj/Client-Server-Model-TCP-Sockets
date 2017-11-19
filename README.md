# client-server-model-tcp-sockets
Demonstration of the client server model using TCP sockets.

This program demonstrates the client-server model using TCP socket programming. A server can be started and then clients can login to that server and begin sending messages back and forth. All messages flow through the server and are then passed on to the client.

Both the Client and Server have specific commands that they can use to perform functions.

## Compilation
Use provided makefile

## Running the program:
The program can be run in two modes, client or server with the c or s flag. Each mode requires a port number to be specified.

Example: ./client-server c 5000  
This will run a client on port 5000  

Example: ./client-server s 5000  
This will run a server on port 5000  

## How to log in to server
LOGIN [SERVER IP ADDRESS] [PORT NUMBER]  
LOGIN 192.168.1.50 5000  

## How to send a message to another client
SEND [CLIENT IP ADDRESS] [MESSAGE]  
SEND 192.168.1.30 TEST MESSAGE  

## Available Client Commands

## Available Server Commands


