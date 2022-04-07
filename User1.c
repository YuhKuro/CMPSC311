#include <stdio.h>

#include <sys/socket.h>

#include <stdlib.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <string.h>





struct sockaddr_in serv; // Main Socket Variable

int fd; // File Descriptor for the socket

int conn; //This is the connection file descriptor which will be used to retrieve client connections

char message[100]; // Stores the messages being sent by the server

char header[500] = "User1: "; // Assigns the tag User1 to the message to identify who is sending the message



int main()

{

    fd = socket(AF_INET, SOCK_STREAM, 0); // Saves the sockets file descriptor to a variable fd

    serv.sin_family = AF_INET; // Designates the address family to internet protocol

    serv.sin_port = htons(18000); // Connects to the sockets port

    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr); //This binds the client to localhost

    connect(fd, (struct sockaddr *)&serv, sizeof(serv)); //This connects the client to the server.

    while(1) {

        printf("Enter a message: ");

        fgets(message, 100, stdin); // Gets message input from user in console

        strcat(header, message); // Puts User tag infront of message

        send(fd, header, strlen(message) + strlen(header), 0); // Sends message to the socket

        memset(&header, 0,  sizeof(message)); // clear message sent buffer

        strcpy(header, "User1: "); // reset to default message
    }

}