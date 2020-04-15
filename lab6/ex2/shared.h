#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define CLIENT_SERVER_STOP 5
#define CLIENT_SERVER_DISCONNECT 4
#define CLIENT_SERVER_LIST 3
#define CLIENT_SERVER_CONNECT 2
#define CLIENT_SERVER_INIT 1

#define SERVER_CLIENT_CHAT_INIT 1
#define SERVER_CLIENT_TERMINATE 2
#define SERVER_CLIENT_REGISTRED 3
#define CLIENT_CLIENT_MSG 4
#define CLIENT_CLIENT_DICONNECT 5

#define SERVER_MAX_CLIENTS_CAPACITY 32
#define MAX_MSG_LENGTH 256

#define QUEUE_GENERATOR_PREFIX ("/queues-")
#define QUEUE_SERVER_NAME (concat(QUEUE_GENERATOR_PREFIX, "server"))
#define QUEUE_CLIENT_RANDOM_NAME                                               \
  (concat(QUEUE_GENERATOR_PREFIX, concat("client-", randomString(12))))

#define DELETE_QUEUE(name) (mq_unlink(name))
#define CLOSE_QUEUE(descr) (mq_close(descr))
#define CREATE_QUEUE(name) (createQueue(name))
#define GET_QUEUE(name) (mq_open(name, O_WRONLY))

#define SEND_MESSAGE(desc, msgPointer, type)                                   \
  (mq_send(desc, msgPointer, strlen(msgPointer), type))
#define RECEIVE_MESSAGE(desc, msgPointer, typePointer)                         \
  (mq_receive(desc, msgPointer, MAX_MSG_LENGTH, typePointer))
#define REGISTER_NOTIFICATION(desc, sigevent) (mq_notify(desc, sigevent))

void  printError();
int   stringEq(char* str1, char* str2);
char* randomString(int length);
char* concat(const char* s1, const char* s2);
char* getCurrentWorkingDirectory();
int   createQueue(char* name);

struct Client {
  int   clientId;
  int   queueDesc;
  char* name;
  int   available;
} typedef Client;
