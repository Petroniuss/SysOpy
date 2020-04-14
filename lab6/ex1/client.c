#include "shared.h"
#include <signal.h>

int   clientQueueId;
int   serverQueueId;
int   clientId;
key_t key;

// When chatting
int chateeQueueId = -1;

void exitClient() {
  printf("Client -- exit..\n");
  exit(EXIT_SUCCESS);
}

// SEND - INIT
void registerMe() {
  ClientServerMessage* csMsg = malloc(sizeof(ClientServerMessage));
  csMsg->clientKey = key;
  csMsg->type = CLIENT_SERVER_INIT;

  SEND_MESSAGE(serverQueueId, csMsg);
  printError();

  ServerClientMessage* scMsg = malloc(sizeof(ServerClientMessage));
  RECEIVE_MESSAGE(clientQueueId, scMsg, SERVER_CLIENT_REGISTRED);
  printError();
  clientId = scMsg->clientId;
  printf("Client -- registered with id: %d\n", clientId);

  free(csMsg);
  free(scMsg);
}
// ----------------

// SEND - STOP
void sendStop() {
  printf("Client -- sending STOP..\n");
  ClientServerMessage* msg = malloc(sizeof(ClientServerMessage));
  msg->clientId = clientId;
  msg->type = CLIENT_SERVER_STOP;

  SEND_MESSAGE(serverQueueId, msg);
  free(msg);
  exitClient();
}

void handleExitSignal(int sig) { sendStop(); }

// ----------------

// SEND - LIST
void sendList() {
  printf("Client -- sending LIST..\n");
  ClientServerMessage* msg = malloc(sizeof(ClientServerMessage));
  msg->type = CLIENT_SERVER_LIST;
  msg->clientId = clientId;

  SEND_MESSAGE(serverQueueId, msg);
  free(msg);
}
// ----------------

// SEND - DISCONNECT
void sendDisconnect() {
  printf("Client -- sending DISCONNECT\n");
  ClientServerMessage* msg = malloc(sizeof(ClientServerMessage));
  msg->clientId = clientId;
  msg->type = CLIENT_SERVER_DISCONNECT;

  SEND_MESSAGE(serverQueueId, msg);
  free(msg);
  if (chateeQueueId != -1) {
    ClientClientMessage* ccMsg = malloc(sizeof(ClientClientMessage));
    ccMsg->type = CLIENT_CLIENT_DICONNECT;

    SEND_MESSAGE(chateeQueueId, ccMsg);
    chateeQueueId = -1;
    free(ccMsg);
  }
}
// ----------------

// SEND - CONNECT
void sendConnect(int chateeId) {
  printf("Client -- seending CONNECT to chatee with ID: %d\n", chateeId);
  ClientServerMessage* msg = malloc(sizeof(ClientServerMessage));
  msg->clientId = clientId;
  msg->chateeId = chateeId;
  msg->type = CLIENT_SERVER_CONNECT;

  SEND_MESSAGE(serverQueueId, msg);
  free(msg);
  ServerClientMessage* scMsg = malloc(sizeof(ServerClientMessage));
  RECEIVE_MESSAGE(clientQueueId, scMsg, SERVER_CLIENT_CHAT_INIT);

  chateeQueueId = GET_QUEUE(scMsg->chateeKey);
  printError();
  printf("Client -- started chat with client: %d, key: %d\n", chateeId,
         scMsg->chateeKey);
}
// ----------------

// SEND - MSG
void sendMessage(char* message) {
  ClientClientMessage* msg = malloc(sizeof(ClientClientMessage));
  msg->type = CLIENT_CLIENT_MSG;

  SEND_MESSAGE(chateeQueueId, msg);
}
// ----------------

// HANDLE - DISCONNECT
void handleDisconnect(ClientClientMessage* msg) {
  printf("Client received disconnect msg from chatee..\n");
  chateeQueueId = -1;
  sendDisconnect();
}
// ----------------

// Handle - CHAT_INIT
void handleChatInit(ServerClientMessage* msg) {
  printf("Client enters chat..\n");
  chateeQueueId = GET_QUEUE(msg->chateeKey);
  printError();
}
// ----------------

// Handle - TERMINATE
void handleTerminate() {
  printf("Received terminate signal.. Server is wating for STOP..\n");
}
// ----------------

// Handle - MSG
void handleMessage(ClientClientMessage* msg) {
  printf("Client recieved msg..\n");
  printf("\t%s\n", msg->msg);
}
// ----------------

void execuateAtExit() { DELETE_QUEUE(clientQueueId); }

int main(int charc, char* argv[]) {
  clientQueueId = CREATE_QUEUE(UNIQUE_KEY);
  serverQueueId = GET_QUEUE(SERVER_KEY);

  signal(SIGINT, handleExitSignal);
  atexit(execuateAtExit);

  registerMe();

  char buffer[64];
  char message[MAX_MSG_LENGTH];
  while (1) {
    //   First handle waiting messages from client/server.
    while (!isQueueEmpty(clientQueueId)) {
      ServerClientMessage* scMsg = malloc(sizeof(ServerClientMessage));
      ClientClientMessage* ccMsg = malloc(sizeof(ClientClientMessage));

      // Handle messages from server
      if (RECEIVE_MESSAGE_NO_WAIT(clientQueueId, scMsg, 0) != -1) {
        if (scMsg->type == SERVER_CLIENT_CHAT_INIT) {
          handleChatInit(scMsg);
        } else if (scMsg->type == SERVER_CLIENT_TERMINATE) {
          handleTerminate();
        }
        // Handle messages from another client
      } else if (RECEIVE_MESSAGE_NO_WAIT(clientQueueId, ccMsg, 0) != -1) {
        if (ccMsg->type == CLIENT_CLIENT_MSG) {
          handleMessage(ccMsg);
        } else if (ccMsg->type == CLIENT_CLIENT_DICONNECT)
          handleDisconnect(ccMsg);
      }
    }

    // Secondly, client executes command.
    scanf("%s", buffer);
    if (stringEq(buffer, "STOP")) {
      sendStop();
    } else if (stringEq(buffer, "LIST")) {
      sendList();
    } else if (stringEq(buffer, "DISCONNECT")) {
      sendDisconnect();
    } else if (stringEq(buffer, "CONNECT")) {
      int chateeId;

      scanf("%d", &chateeId);
      sendConnect(chateeId);
    } else if (stringEq(buffer, "SEND")) {
      scanf("%s", message);

      if (chateeQueueId == -1) {
        printf("Client -- unable to send message\n");
      } else {
        sendMessage(message);
      }
    } else if (stringEq(buffer, "PASS")) {
      ;
    } else {
      printf("Client --  unknown command.\n");
    }
  }

  return 0;
}