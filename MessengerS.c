#include <stdio.h>

#include <sys/socket.h> //For Sockets

#include <stdlib.h>

#include <netinet/in.h> //For the AF_INET (Address Family)

#include <unistd.h>

#include <fcntl.h>

#include <errno.h>

#include <string.h>

#include <pthread.h>

#include <sys/types.h>

#include <arpa/inet.h>

#include <signal.h>

#include "clients.h" // For CLientList function definitions

#define LENGTH_MSG 101

#define LENGTH_SEND 201

#define LENGTH_NAME 31




int fd = 0, client_fd = 0; // File Descriptors for the sockets

ClientList *root, *now; // List of currently connected clients

void catch_ctrl_c_and_quit(int sig) {
  ClientList *tmp;

  while (root != NULL) {
    printf("\nClose socket: %d\n", root->data);
    close(root->data);
    tmp = root;
    root = root->link;
    free(tmp);
  }
  printf("Closing the server\n");

  exit(EXIT_SUCCESS);
}

void broadcast_message(ClientList *np, char tmp_buf[]) {
  ClientList *tmp = root->link;

  while (tmp != NULL) {
    if (np->data != tmp->data) {
      printf("Send to socket %d: \"%s\" \n", tmp->data, tmp_buf);

      send(tmp->data, tmp_buf, LENGTH_SEND, 0);
    }

    tmp = tmp->link;
  }
}

void client_handler(void *p_client) {
  int leave_flag = 0;

  // Initialize message buffers
  char nickname[LENGTH_NAME] = {};
  char recv_buf[LENGTH_MSG] = {};
  char send_buf[LENGTH_SEND] = {};

  ClientList *np = (ClientList *) p_client;

  if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME -1) {
    printf("%s didn't input name.\n", np->ip);

    leave_flag = 1;
  } else {
    strncpy(np->name, nickname, LENGTH_NAME);

    printf("%s(%s)(%d) join the room.\n", np->name, np->ip, np->data);

    sprintf(send_buf, "%s(%s) has joined the room.", np->name, np->ip);

    broadcast_message(np, send_buf);
  }
  
  while (1) {
    if (leave_flag) {
      break;
    }

    int receive = recv(np->data, recv_buf, LENGTH_MSG, 0);
    if (receive > 0) {
      if (strlen(recv_buf) == 0) {
        continue;
      }
      sprintf(send_buf, "%s: %s from %s", np->name, recv_buf, np->ip);
    } else if(receive == 0 || strcmp(recv_buf, "exit") == 0) {
      printf("%s(%s)(%d) leaves the room.\n", np->name, np->ip, np->data);
      sprintf(send_buf, "%s(%s) has left the room.", np->name, np->ip);
      leave_flag = 1;
    } else {
      printf("An error occurred when trying to send the message\n");
      leave_flag = 1;
    }

    broadcast_message(np, send_buf);
  }

  close(np->data);

  if (np == now) {
    now = np->prev;
    now->link = NULL;
  } else {
    np->prev->link = np->link;
    np->link->prev = np->prev;
  }

  free(np);
}

int main()

{

    signal(SIGINT, catch_ctrl_c_and_quit);

    fd = socket(AF_INET, SOCK_STREAM, 0); // This creates a new socket and returns the identifier of the socket into fd

    if(fd < 0) {  // Checks to see if there was an error creating the socket
      perror("Socket Error");
      exit(EXIT_FAILURE);
    } 


    struct sockaddr_in serv, client; // Main Sockets Variables

    int s_addrlen = sizeof(serv);
    int c_addrlen = sizeof(client);

    memset(&serv, 0, s_addrlen);
    memset(&client, 0, c_addrlen);

    serv.sin_family = PF_INET;

    serv.sin_addr.s_addr = INADDR_ANY;

    serv.sin_port = htons(18000); // Defines the port at where the server will listen for connections

    bind(fd, (struct sockaddr *)&serv, s_addrlen); //Assigns the address specified by serv to the socket

    listen(fd, 5); // Listens for client connections only allows 5

    // Returns the current address to which the socket sockfd is bound, in the buffer pointed to by addr.
    getsockname(fd, (struct sockaddr*) &serv, (socklen_t*) &s_addrlen); 
    printf("Start Server on: %s:%d\n", inet_ntoa(serv.sin_addr), ntohs(serv.sin_port));

    root = newClient(fd, inet_ntoa(serv.sin_addr));
    now = root;

    while(1){
        
        client_fd = accept(fd, (struct sockaddr*) &client, (socklen_t*) &c_addrlen); // Keeps accepting new clients

        getpeername(client_fd, (struct sockaddr*) &client, (socklen_t*) &c_addrlen);      
        printf("Client %s:%d has connected.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        ClientList *c = newClient(client_fd, inet_ntoa(client.sin_addr));

        c->prev = now;
        now->link = c;
        now = c;

        pthread_t new_client;

        if (pthread_create(&new_client, NULL, (void *) client_handler, (void *) c) != 0) {
          perror("An error occured when attempting to create a new thread");

          exit(EXIT_FAILURE);
        }

    }

    return 0;

}
