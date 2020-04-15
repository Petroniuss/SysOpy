#include "shared.h"
#include <signal.h>

int   clientQueueDesc;
int   serverQueueDesc;
int   clientId;
char* name;

// When chatting
int chateeQueueDesc = -1;

void exitClient() {
  printf("Client -- exit..\n");
  CLOSE_QUEUE(serverQueueDesc);
  CLOSE_QUEUE(clientQueueDesc);

  DELETE_QUEUE(name);
  exit(EXIT_SUCCESS);
}

// SEND - INIT
void registerMe() {
  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %s", CLIENT_SERVER_INIT, name);

  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_SERVER_INIT);

  char scMsg[MAX_MSG_LENGTH];
  RECEIVE_MESSAGE(clientQueueDesc, scMsg, SERVER_CLIENT_REGISTRED);

  int type;
  sscanf(scMsg, "%d %d", &type, &clientId);
  printf("Client -- registered with id: %d, key: %s\n", clientId, name);
}
// ----------------

// SEND - STOP
void sendStop() {
  printf("Client -- sending STOP..\n");

  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %d", CLIENT_SERVER_STOP, clientId);
  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_SERVER_STOP);

  exitClient();
}

void handleExitSignal(int sig) { sendStop(); }
// ----------------

// SEND - LIST
void sendList() {
  printf("Client -- sending LIST..\n");

  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %d", CLIENT_SERVER_LIST, clientId);
  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_SERVER_LIST);
}
// ----------------

// SEND - DISCONNECT
void sendDisconnect() {
  printf("Client -- DISCONNECT\n");

  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %d", CLIENT_SERVER_DISCONNECT, clientId);
  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_SERVER_DISCONNECT);

  if (chateeQueueDesc != -1) {
    char ccMsg[MAX_MSG_LENGTH];
    sprintf(csMsg, "%d %d", CLIENT_CLIENT_DICONNECT, clientId);
    SEND_MESSAGE(serverQueueDesc, ccMsg, CLIENT_CLIENT_DICONNECT);

    CLOSE_QUEUE(chateeQueueDesc);
    chateeQueueDesc = -1;
  }
}
// ----------------

// SEND - CONNECT
void sendConnect(int chateeId) {
  printf("Client -- seending CONNECT to chatee with ID: %d\n", chateeId);

  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %d %d", CLIENT_SERVER_CONNECT, clientId, chateeId);
  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_SERVER_CONNECT);
}
// ----------------

// SEND - MSG
void sendMessage(char* message) {
  char csMsg[MAX_MSG_LENGTH];
  sprintf(csMsg, "%d %s", CLIENT_CLIENT_MSG, message);
  SEND_MESSAGE(serverQueueDesc, csMsg, CLIENT_CLIENT_MSG);
}
// ----------------

// HANDLE - DISCONNECT
void handleDisconnect(char* msg) {
  printf("Client -- received disconnect msg from chatee..\n");

  chateeQueueDesc = -1;
  CLOSE_QUEUE(chateeQueueDesc);

  sendDisconnect();
}
// ----------------

// Handle - CHAT_INIT
void handleChatInit(char* msg) {
  int  type;
  char chateeName[MAX_MSG_LENGTH];
  sprintf(msg, "%d %d", &type, &chateeName);
  printf("Client -- entering chat \n");
  chateeQueueDesc = GET_QUEUE(chateeQueueDesc);
}
// ----------------

// Handle - TERMINATE
void handleTerminate() {
  printf("Client -- Received terminate signal.. Server is wating for STOP..\n");
}
// ----------------

// Handle - MSG
void handleMessage(char* ccMsg) {
  char msg[MAX_MSG_LENGTH];
  int  type;
  sprintf(ccMsg, "%d %s", &type, msg);

  printf("Client -- recieved msg..\n");
  printf("------------------------------------------------\n");
  printf("\t%s\n", msg);
  printf("------------------------------------------------\n");
}
// ----------------

void registerNotification() {
  struct sigevent ev;
  ev.sigev_notify = SIGEV_SIGNAL;
  ev.sigev_signo = SIGUSR1;

  REGISTER_NOTIFICATION(clientQueueDesc, &ev);
}

void handleSignal(int signal) {

  char msg[MAX_MSG_LENGTH];
  int  type;

  RECEIVE_MESSAGE(clientQueueDesc, msg, &type);
  if (type == SERVER_CLIENT_CHAT_INIT) {
    handleChatInit(msg);
  } else if (type == SERVER_CLIENT_TERMINATE) {
    handleTerminate();
  } else if (type == CLIENT_CLIENT_MSG) {
    handleMessage(msg);
  } else if (type == CLIENT_CLIENT_DICONNECT) {
    handleDisconnect(msg);
  } else {
    printf("Client -- received message of unknown type..\n");
  }

  registerNotification();
}

int main(int charc, char* argv[]) {
  crand(time(NULL));
  name = QUEUE_RANDOM_NAME;
  clientQueueDesc = CREATE_QUEUE(name);
  // This is just in case our key was not unique.
  if (clientQueueDesc == -1) {
    printf("There already exists client associated with this queue...\n");
    printError();
    return -1;
  }

  serverQueueDesc = GET_QUEUE(QUEUE_SERVER_PATH);

  signal(SIGINT, handleExitSignal);
  signal(SIGUSR1, handleSignal);

  registerMe();
  registerNotification();

  char buffer[MAX_MSG_LENGTH];
  char message[MAX_MSG_LENGTH];

  while (1) {
    // Client executes command.
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

      if (chateeQueueDesc == -1) {
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