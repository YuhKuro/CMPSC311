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
#include <fcntl.h>
#include <sys/stat.h>

#define LENGTH_MSG 101
#define LENGTH_SEND 201
#define LENGTH_NAME 31

int fd = 0; // File Descriptor for the socket
int fd_GUI_SEND = 0; // File Descriptor to send to python gui
int fd_GUI_RECV = 0; // Used to recieve messages sent from python
int fd_EXIT = 0; // Used for python exit
char message[10000]; // Stores the messages being sent by the server
char header[500] = "User1: "; // Assigns the tag User1 to the message to identify who is sending the message
char nickname[LENGTH_NAME] = {};
volatile sig_atomic_t flag = 0;

void handle_quit(int sig) {
  flag = 1;
}

void handle_incoming_message() {
  char recv_msg[LENGTH_SEND] = {};
  mkfifo("ctopy", 0666);

  while (1) {
    int receive = recv(fd, recv_msg, LENGTH_SEND, 0);
    fd_GUI_SEND = open("ctopy", O_WRONLY);
    if (receive > 0 && strlen(recv_msg) > 0) {
      write(fd_GUI_SEND, recv_msg, strlen(recv_msg));
      printf("\r%s\n", recv_msg);
      printf("\r%s", "> ");
      fflush(stdout);
    } else if (receive == 0) {
      break;
    }
    close(fd_GUI_SEND);
  }
}

void handle_new_message() {
  char msg[LENGTH_MSG] = {};
  int count = 0;

  while (1) {
    printf("\r%s", "> ");
    fflush(stdout);
    fd_GUI_RECV = open("pytoc", O_RDONLY); // Opens the PIPE created by Python GUI for reading
    count = read(fd_GUI_RECV, msg, LENGTH_MSG); // Reads in data sent through PIPE to msg buffer
    if(count > 0) // Checks to make sure data size from PIPE wasnt 0 or less
    	send(fd, msg, LENGTH_MSG, 0); // Sends message to socket
    close(fd_GUI_RECV);
    memset(&msg, 0, sizeof(msg));
  }
}

void handle_exit(){
  char exitMSG[6] = {};
  int count = 0;
  while(1)
  {
    fd_EXIT = open("onexit", O_RDONLY); // Checks to see if a exit command was sent from python
    count = read(fd_EXIT, exitMSG, 6);
    if (strcmp(exitMSG, "exit") == 0) {
      close(fd_EXIT);
      break;
    }
    close(fd_EXIT);
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
  pthread_t exit_thread;

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
  serv.sin_addr.s_addr = inet_addr("178.128.17.25");
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
  
  if (pthread_create(&exit_thread, NULL, (void *) handle_exit, NULL) != 0){
    printf("Failed to create exit thread\n");
    
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
