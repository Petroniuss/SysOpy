#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define CLIENT_SERVER_STOP 1
#define CLIENT_SERVER_DISCONNECT 2
#define CLIENT_SERVER_LIST 3
#define CLIENT_SERVER_CONNECT 4
#define CLIENT_SERVER_INIT 5

#define CLIENT_CLIENT_MSG 1
#define CLIENT_CLIENT_DICONNECT 2

#define SERVER_CLIENT_CHAT_INIT 1
#define SERVER_CLIENT_TERMINATE 2
#define SERVER_CLIENT_REGISTRED 3

#define SERVER_MAX_CLIENTS_CAPACITY 32
#define SERVER_MESSAGE_TYPE_PRIORITY (-6)
#define MAX_MSG_LENGTH 128

#define KEY_GENERATOR_PATH (getenv("HOME"))
#define PROJECT_IDENTIFIER (112358)
const int ID_NUMS[SERVER_MAX_CLIENTS_CAPACITY];

#define UNIQUE_KEY (ftok(KEY_GENERATOR_PATH, getpid()))
#define SERVER_KEY (ftok(KEY_GENERATOR_PATH, PROJECT_IDENTIFIER))

#define DELETE_QUEUE(id) (msgctl(id, IPC_RMID, NULL))
#define CREATE_QUEUE(key) (msgget(key, 0666 | IPC_CREAT | IPC_EXCL))
#define GET_QUEUE(key) (msgget(key, 0666))

#define SEND_MESSAGE(id, msgPointer)                                           \
  (msgsnd(id, msgPointer, sizeof(*msgPointer), 0))
#define RECEIVE_MESSAGE(id, msgPointer, type)                                  \
  (msgrcv(id, msgPointer, sizeof(*msgPointer), type, 0))
#define RECEIVE_MESSAGE_NO_WAIT(id, msgPointer, type)                          \
  (msgrcv(id, msgPointer, sizeof(*msgPointer), type, IPC_NOWAIT))

struct ClientServerMessage {
  long  type;
  int   clientId;
  key_t clientKey;
  int   chateeId;
} typedef ClientServerMessage;

struct ClientClientMessage {
  long type;
  char msg[MAX_MSG_LENGTH];
} typedef ClientClientMessage;

struct ServerClientMessage {
  long  type;
  int   clientId;
  key_t chateeKey;
} typedef ServerClientMessage;

struct Client {
  key_t key;
  int   clientId;
  int   queueId;
  int   available;
} typedef Client;

void printError();
int  stringEq(char* str1, char* str2);
int  isQueueEmpty(int queueId);
int  iPointer;