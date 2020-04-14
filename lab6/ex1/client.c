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

// INIT
void registerMe() {
  ClientServerMessage csMsg;
  csMsg.clientKey = key;
  csMsg.type = CLIENT_SERVER_INIT;

  SEND_MESSAGE(serverQueueId, &csMsg);
  printError();

  ServerClientMessage scMsg;
  RECEIVE_MESSAGE(clientQueueId, &scMsg, SERVER_CLIENT_REGISTRED);
  printError();
  clientId = scMsg.clientId;
}
// ----------------

int main(int charc, char* argv[]) {
  key = UNIQUE_KEY;
  clientQueueId = CREATE_QUEUE(key);
  serverQueueId = GET_QUEUE(SERVER_KEY);

  registerMe();
  return 0;
}