# Simple Group Chat Simulation
This is a raw copy of the Simple Group Chat project  

## In this project...
- Used INET TCP protocol to set the group chat environment using C in a Linux virtual machine 
- Programmed and compiled group chat server and user client using cmake
- Simulated multiple user clients to connect, send, and receive messages through the server

## About:
- Messages are randomly generated at a random time by each active client and will be sent to the server
- Messages received by the server will be sent and displayed on all clients' terminals in order
- Server is designed to automatically shutdown when all clients are signaled to exit

## Future plan:
- Modify so that it takes user input as the message
- Allow users to write a username and let it be displayed along with the messages
