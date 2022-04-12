#ifndef LIST
#define LIST

typedef struct Node {
  int data;
  struct Node* prev;
  struct Node* link;
  char ip[16];
  char name[31];
} ClientList;

ClientList *newClient(int socket, char* ip) {
  ClientList *client_list = (ClientList *) malloc(sizeof(ClientList));

  client_list->data = socket;

  client_list->prev = NULL;

  client_list->link = NULL;

  strncpy(client_list->ip, ip, 16);

  strncpy(client_list->name, "NULL", 5);

  return client_list;
}

#endif // LIST
