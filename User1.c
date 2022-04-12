#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define LENGTH_MSG 101
#define LENGTH_SEND 201
#define LENGTH_NAME 31

int fd = 0; // File Descriptor for the socket
char message[100]; // Stores the messages being sent by the server
char header[500] = "User1: "; // Assigns the tag User1 to the message to identify who is sending the message
char nickname[LENGTH_NAME] = {};
volatile sig_atomic_t flag = 0;

void handle_quit(int sig) {
  flag = 1;
}

void handle_incoming_message() {
  char recv_msg[LENGTH_SEND] = {};

  while (1) {
    int receive = recv(fd, recv_msg, LENGTH_SEND, 0);

    if (receive > 0) {
      printf("\r%s\n", recv_msg);
      printf("\r%s", "> ");
      fflush(stdout);
    } else if (receive == 0) {
      break;
    }
  }
}

void handle_new_message() {
  char msg[LENGTH_MSG] = {};

  while (1) {
    printf("\r%s", "> ");
    fflush(stdout);

    while (fgets(msg, LENGTH_MSG, stdin) != NULL) {
      int i;
      for (i = 0; i < LENGTH_MSG; i++) {
        if (msg[i] == '\n') {
          msg[i] = '\0';
          break;
        }
      }
      
      if (strlen(msg) == 0) {
        printf("\r%s", "> ");
        fflush(stdout);
      } else {
        break;
      }
    }

    send(fd, msg, LENGTH_MSG, 0);

    if (strcmp(msg, "exit") == 0) {
      break;
    }
  }

  handle_quit(2);
}


int main() {
  signal(SIGINT, handle_quit);

  printf("Please enter a username: ");
  if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {

    int i;
    for (i = 0; i < LENGTH_NAME; i++) {
      if (nickname[i] == '\n') {
        nickname[i] = '\0';
        break;
      }
    }
  }

  if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME - 1) {
    printf("\nName should be between 2 and 31 characters long.\n");

    exit(EXIT_FAILURE);
  }

  pthread_t send_msg_thread;
  pthread_t recv_msg_thread;

  fd = socket(AF_INET, SOCK_STREAM, 0); // Saves the sockets file descriptor to a variable fd

  if (fd == -1) {
    printf("Failed to create a socket\n");

    exit(EXIT_FAILURE);
  }

  struct sockaddr_in serv, client; // Main Socket Variable

  int s_addrlen = sizeof(serv);
  int c_addrlen = sizeof(client);

  memset(&serv, 0, s_addrlen);
  memset(&client, 0, c_addrlen);

  serv.sin_family = PF_INET; // Designates the address family to internet protocol
  serv.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv.sin_port = htons(18000); // Connects to the sockets port

  int conn; //This is the connection file descriptor which will be used to retrieve client connections
  conn = connect(fd, (struct sockaddr *)&serv, s_addrlen); //This connects the client to the server.

  if (conn == -1) {
    printf("Failed to connect to server\n");

    exit(EXIT_FAILURE);
  }

  getsockname(fd, (struct sockaddr*) &client, (socklen_t*) &c_addrlen);
  getpeername(fd, (struct sockaddr*) &serv, (socklen_t*) &s_addrlen);

  printf("Connect to Server: %s:%d\n", inet_ntoa(serv.sin_addr), ntohs(serv.sin_port));
  printf("You are: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

  send(fd, nickname, LENGTH_NAME, 0);

  if (pthread_create(&send_msg_thread, NULL, (void *) handle_new_message, NULL) != 0) {
    printf("Failed to create send message thread\n");

    exit(EXIT_FAILURE);
  }

  if (pthread_create(&recv_msg_thread, NULL, (void *) handle_incoming_message, NULL) != 0) {
    printf("Failed to create receive message thread\n");

    exit(EXIT_FAILURE);
  }

  while(1) {
    if (flag) {
      printf("\nDisconnecting...\n");
      break;
    }
  }

  close(fd);

  return 0;
}
