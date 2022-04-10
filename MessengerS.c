#include <stdio.h>

#include <sys/socket.h> //For Sockets

#include <stdlib.h>

#include <netinet/in.h> //For the AF_INET (Address Family)

#include <unistd.h>

#include <fcntl.h>

#include <errno.h>

#include <string.h>





struct sockaddr_in serv; // Main Socket Variable

int fd; // File Descriptor for the socket

int conn; //This is the connection file descriptor which will be used to retrieve client connections

char message[100]; // Stores the messages being sent by the server



int main()

{

    serv.sin_family = AF_INET;

    serv.sin_port = htons(18000); // Defines the port at where the server will listen for connections

    serv.sin_addr.s_addr = INADDR_ANY;

    int flags; // Used for making the socket non blocking



    fd = socket(AF_INET, SOCK_STREAM, 0); // This creates a new socket and returns the identifier of the socket into fd



    if(fd < 0) // Checks to see if there was an error creating the socket

        perror("Socket Error");

    bind(fd, (struct sockaddr *)&serv, sizeof(serv)); //Assigns the address specified by serv to the socket

    listen(fd, 5); // Listens for client connections only allows 5

    while(conn = accept(fd, (struct sockaddr *)NULL, NULL)){

        pid_t pid;

        if((pid = fork()) == 0){

            while (recv(conn, message, 500, 0)>0){

                printf("%s",message);

                send(conn, message, sizeof(message), 0); // Sends recieved message to client

                

            }

            exit(0);

        }

    }

    return 0;

}