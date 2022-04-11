#include <stdio.h>



#include <sys/socket.h>



#include <stdlib.h>



#include <netinet/in.h>



#include <arpa/inet.h>



#include <string.h>

#include <fcntl.h>

#include <unistd.h>

#include <pthread.h>











struct sockaddr_in serv; // Main Socket Variable



int fd; // File Descriptor for the socket

int fd_send, fd_rec; // fd_send stores the text file for sending text to the python GUI. fd_rec opens the text file for recieving text from

// the python GUI



int conn; //This is the connection file descriptor which will be used to retrieve client connections



char message[10000] = ""; // Stores the messages being sent by the server

char message_r[10000] = ""; // Stores the message recieved from the socket



char header[10000] = "User1: "; // Assigns the tag User1 to the message to identify who is sending the message



static void *readMessage() // Reads messages from socket and sends them to Python GUI

{

    FILE *file; // Creates a file pointer

    recv(fd, message_r, 10000, 0); // Recieves the message sent from the socket and stores it in message_r char array

    file = fopen("send.txt", "a"); //Opens the text file for communicating with python GUI and stores the pointer to the file in the file variable

    if(file != NULL) // Checks to make sure file exists

    {

	    fwrite(message_r, 1, strlen(message_r), file); // Writes message that is stored in message_r char array to the file

	    memset(&message_r, 0, sizeof(message_r)); // Clears message_r for new messages

    }

    fclose(file); // Closes the file

    return (void*) "done"; // indicates thread process has been completed



}



static void *writeMessage() // Reads messages from Python GUI and sends them to socket

{

    int count; // Stores number of bytes read in from PIPE

    //fd_rec = open("recieve.txt", O_RDONLY, 0777);

    fd_rec = open("pytoc", O_RDONLY); // Opens the PIPE in read only mode and stores the file descriptor in fd_rec

    

    if(fd_rec > 0) // Checks to make sure there was no errors when opening PIPE

    {

        count = read(fd_rec, message, 10000); // Reads in strings sent through PIPE and saves them in message char array

        if(count != 0) // Checks to make sure the bytes read in are not 0

        {

            strcat(header, message); // Puts User tag infront of message



            send(fd, header, strlen(message) + strlen(header), 0); // Sends message to the socket



            memset(&header, 0,  sizeof(header)); // clear message sent buffer

            

            memset(&message, 0, sizeof(message)); // clear message buffer



            strcpy(header, "User1: "); // reset to default message 

            

        }

        memset(&header, 0,  sizeof(message)); // clear message sent buffer

        strcpy(header, "User1: "); // reset to default message

        close(fd_rec);

        //fclose(fopen("recieve.txt", "w"));

    }

    return (void*) "done"; // Indicates when the write message thread has completed

}



int main()



{

    pthread_t t1, t2; // t1 is the write thread and t2 is the read thread

    int s,t;

    void *res;

    fd = socket(AF_INET, SOCK_STREAM, 0); // Saves the sockets file descriptor to a variable fd



    serv.sin_family = AF_INET; // Designates the address family to internet protocol



    serv.sin_port = htons(18000); // Connects to the sockets port



    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr); //This binds the client to localhost



    connect(fd, (struct sockaddr *)&serv, sizeof(serv)); //This connects the client to the server.



    while(1) {



        s = pthread_create(&t1, NULL, writeMessage, NULL); // Creates thread that sends message to socket

        if(s == -1)

        {

            perror("Write Thread Error");

        }

        s=pthread_join(t1, &res); // Pauses main thread until the writing thread has completed

        t = pthread_create(&t2, NULL, readMessage, NULL);  // Creates thread that reads messages from socket and sends them to python GUI through text file might change those so it is sent through PIPE if there is problems when sending other users messages

        if(t == -1)

        {

            perror("Read Thread Error");

        }

        t=pthread_join(t2, &res);

    }



}
