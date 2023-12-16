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

Compile by typing this command to the terminal:
- $make 
Run by typing this command to the terminal:
$./server \n 
> Server listening on port 42685 \n 
$./client fraenkel 42685  

Note: Run $./client fraenkel 42685 on 2 different machines to play together 

To play the game, connect 2 machines to the server, the game starts only when exactly 2 clients are connected. Use w and s to move your paddle up and down. 
If you're the first player connected, you will control the left paddle, otherwise you will control the right paddle. (It's experiencing lag for the paddle right now bc of the sleep_ms was not correctly placed.
But the message when through and controlled the game, it's just sleeping for too long)

Playable (test) file: pong.c
- move by 'w' and 's'

Compile and run:
- $make 
- $./pong
