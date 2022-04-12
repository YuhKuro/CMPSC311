#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

#define PORT 8888 
     
int main()  
{  
    int opt = 1;  
    int mainSocket , addrlen , new_socket , client_sockets[5] , clientMax = 5 , notification, i , valMessage , fd, j;  
    int max_fd;  
    struct sockaddr_in address;  
         
    char buffer[5000];  //message buffer of 5000 
         
    //set of socket descriptors 
    fd_set readfds;   
     
    //initialise all client_sockets[] to 0 so not checked 
    for (i = 0; i < clientMax; i++)  
    {  
        client_sockets[i] = 0;  
    }  
         
    //create a main socket 
    if( (mainSocket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("Main Socket Error");  
        exit(EXIT_FAILURE);  
    }  
     
    //Allows main socket to have multiple connections
    if( setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("Main Socket Multi Conn");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created IPV4
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (bind(mainSocket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(mainSocket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(1)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add main socket to set 
        FD_SET(mainSocket, &readfds);  
        max_fd = mainSocket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < clientMax ; i++)  
        {  
            //reads in clients file descriptors to fd 
            fd = client_sockets[i];  
                 
            //if valid socket descriptor then add to read list 
            if(fd > 0)  
                FD_SET( fd , &readfds);  
                 
            // Checks to see if current fd is greater than max then sets max fd equal to fd if so
            if(fd > max_fd)  
                max_fd = fd;  
        }  
     
        //Waits for notification from clients
        notification = select( max_fd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((notification < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
             
        //Checks for new connections
        if (FD_ISSET(mainSocket, &readfds))  
        {  
            if ((new_socket = accept(mainSocket, (struct sockaddr *)&address,
            (socklen_t*)&addrlen))<0)  
            {  
                perror("Error Accepting Client Connection");  
                exit(EXIT_FAILURE);  
            }                          
                 
            //add new socket to array of sockets 
            for (i = 0; i < clientMax; i++)  
            {  
                //if position is empty 
                if( client_sockets[i] == 0 )  
                {  
                    client_sockets[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }  
        }  
             
        // Runs for incoming request from client
        for (i = 0; i < clientMax; i++)  
        {  
            fd = client_sockets[i];
            j =0;  
                 
            if (FD_ISSET(fd,&readfds))  
            {  
                // Checks to see if the client exited also reads incoming client message 
                if ((valMessage = read( fd , buffer, 1024)) == 0)  
                {                        
                    //If client exited we close clients file descriptor and mark position in client_sockets array as empty
                    close( fd );  
                    client_sockets[i] = 0;  
                }  
                     
                //Broadcast message to all clients
                else 
                {  
                    while(client_sockets[j] != 0){
		            buffer[valMessage] = '\0';  
		            send(client_sockets[j] , buffer , strlen(buffer) , 0 );
		            j++;
		            printf("%s", buffer);
                    }  
                }  
            }  
        }  
    }  
         
    return 0;  
} 