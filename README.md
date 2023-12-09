# Networked-Pong
"Networked Pong: Two-Player Synchronized Gameplay" is a project focused on creating a two-player version of Pong that operates over a network. The goal is to combine elements of Networks and Distributed Systems, Files and File System , and Scheduling to enable coordinated gameplay between two players. 

2 files to compile: server.c and client.c

1. client.c
- connect to the server
- constantly send input to server to move the pads
- constantly receive the current deserialized gamestate from the server
- use the received game state to render the game on client side   
2. server.c
- connect at most 2 clients to server
- constantly listening for input from users
- constantly update the game state
- constantly sending the serialized game state to client

Compile and run:
$make 
$./server
$./client computer_name port_number

Playable file: pong.c
- move by 'w' and 's'

Compile and run:
$make 
$./pong
