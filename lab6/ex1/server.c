#include "shared.h"
#include <signal.h>

Client* clients[SERVER_MAX_CLIENTS_CAPACITY];

int serverQueueId = -1;
int clientsRunningCount = 0;
int waitingForClientsToTerminate = 0;
int current = 0;

// FIXME --> convert static allocation to dynamic!

// HANDLE EXIT - CRTL+C
void exitServer() {
  printf("Server exits...\n");

  exit(EXIT_SUCCESS);
}

void handleSignalExit(int signal) {
  ServerClientMessage* msg = malloc(sizeof(ServerClientMessage));
  msg->type = SERVER_CLIENT_TERMINATE;

  if (clientsRunningCount <= 0)
    exitServer();

  for (int i = 0; i < SERVER_MAX_CLIENTS_CAPACITY; i++) {
    if (clients[i]) {
      SEND_MESSAGE(clients[i]->clientId, msg);
    }
  }

  printf("Server waiting for clients to terminate.\n");
}
// -------------------------

// HANDLE - STOP
void handleStop(ClientServerMessage* msg) {
  clients[msg->clientId] = NULL;
  free(clients[msg->clientId]);
  clientsRunningCount -= 1;

  printf("Server -- Received STOP from %d\n", msg->clientId);

  if (waitingForClientsToTerminate <= 0) {
    exitServer();
  }
}
// -------------------------

// HANDLE - DISCONNECT
void handleDisconnect(ClientServerMessage* msg) {
  clients[msg->clientId]->available = 1;
}
// -------------------------

// HANDLE - LIST
void handleList(ClientServerMessage* msg) {
  printf("Server - listing available clients...\n");
  for (int i = 0; i < SERVER_MAX_CLIENTS_CAPACITY; i++) {
    if (clients[i] && clients[i]->available) {
      printf("\tClient --> id - %d, key - %d\n", clients[i]->clientId,
             clients[i]->key);
    }
  }
  printf("Server -- done...\n");
}
// -------------------------

// HANDLE - CONNECT
void handleConnect(ClientServerMessage* msg) {
  int id1 = msg->clientId;
  int id2 = msg->chateeId;

  // Check if client under sent id is avaiable
  if (id2 < 0 || id2 >= SERVER_MAX_CLIENTS_CAPACITY || !clients[id2] ||
      !clients[id2]->available) {
    printf("Server -- requested client is not avaiable\n");
  }

  ClientServerMessage* msg1 = malloc(sizeof(ClientServerMessage));
  ClientServerMessage* msg2 = malloc(sizeof(ClientServerMessage));

  msg1->type = SERVER_CLIENT_CHAT_INIT;
  msg2->type = SERVER_CLIENT_CHAT_INIT;

  msg1->clientKey = clients[id2]->key;
  msg2->clientKey = clients[id1]->key;

  SEND_MESSAGE(clients[id1]->queueId, msg1);
  SEND_MESSAGE(clients[id2]->queueId, msg2);
  printError();

  printf("Server -- initialized chat, %d <=> %d\n", id1, id2);
}
// -------------------------

// HANDLE - INIT
void handleInit(ClientServerMessage* msg) {
  int pointer = -1;
  for (int i = 0; i < SERVER_MAX_CLIENTS_CAPACITY; i++) {
    pointer = (current + i) % SERVER_MAX_CLIENTS_CAPACITY;
    if (!clients[pointer]) {
      break;
    }
  }

  if (pointer == -1) {
    printf("Server -- reached maximum capcity, cannot add another client...\n");
  } else {
    Client* client = malloc(sizeof(Client));
    client->available = 1;
    client->key = msg->clientKey;
    client->clientId = pointer;
    client->queueId = GET_QUEUE(msg->clientKey);
    // debug
    printError();

    clients[pointer] = client;

    // Notify client that he's now registered.
    ServerClientMessage* scMsg = malloc(sizeof(ServerClientMessage));
    scMsg->type = SERVER_CLIENT_REGISTRED;
    scMsg->clientId = pointer;

    SEND_MESSAGE(client->queueId, scMsg);
    printf("Server -- registered client - id: %d, key: %d\n", client->clientId,
           client->key);
    printError();
    free(scMsg);
  }
}
// -------------------------

// RECEIVE MESSAGE
// Note that we handle messages in order based on priority.
void handleMessage() {
  printf("Server -- waiting for message ..\n");
  ClientServerMessage* msg = malloc(sizeof(ClientServerMessage*));

  RECEIVE_MESSAGE(serverQueueId, msg, SERVER_MESSAGE_TYPE_PRIORITY);

  int type = msg->type;
  if (type == CLIENT_SERVER_STOP) {
    handleStop(msg);
  } else if (type == CLIENT_SERVER_DISCONNECT) {
    handleDisconnect(msg);
  } else if (type == CLIENT_SERVER_LIST) {
    handleList(msg);
  } else if (type == CLIENT_SERVER_CONNECT) {
    handleConnect(msg);
  } else if (type == CLIENT_SERVER_INIT) {
    handleInit(msg);
  } else {
    printf("Server -- unknown message type.\n");
  }

  free(msg);
}

void executeAtExit() { DELETE_QUEUE(serverQueueId); }

int main(int argc, char* arrgv[]) {
  serverQueueId = CREATE_QUEUE(SERVER_KEY);
  if (serverQueueId == -1) {
    DELETE_QUEUE(GET_QUEUE(SERVER_KEY));
    serverQueueId = CREATE_QUEUE(SERVER_KEY);
  }
  signal(SIGINT, handleSignalExit);
  atexit(executeAtExit);

  printf("Server running...\n");
  while (1) {
    handleMessage();
  }

  return 0;
}