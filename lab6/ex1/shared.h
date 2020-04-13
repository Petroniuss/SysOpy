#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CLIENT_SERVER_STOP 1
#define CLIENT_SERVER_DISCONNECT 2
#define CLIENT_SERVER_LIST 3
#define CLIENT_SERVER_CONNECT 4
#define CLIENT_SERVER_INIT 5

#define CLIENT_CLIENT_MSG 1
#define CLIENT_CLIENT_DICONNECT 2

#define SERVER_CLIENT_CHAT_INIT 1
#define SERVER_CLIENT_TERMINATE 2

#define SERVER_MAX_CLIENTS_CAPACITY 32
#define MAX_MSG_LENGTH 128

#define KEY_GENERATOR_PATH (getenv("HOME"))
#define PROJECT_IDENTIFIER 'P'

#define UNIQUE_KEY (ftok(KEY_GENERATOR_PATH, PROJECT_IDENTIFIER))
#define SERVER_KEY ((key_t)112358)

#define DELETE_QUEUE(id) (msgctl(id, IPC_RMID, NULL))
#define CREATE_QUEUE(key) (msgget(key, 0666 | IPC_PRIVATE))
#define GET_QUEUE(key) (msgget(key, 0666))

struct ClientServerMessage
{
    long type;
    int clientId;
    key_t clientKey;
    int connectToClientId;
} typedef ClientMessage;

struct ClientClientMessage
{
    long type;
    char msg[MAX_MSG_LENGTH];
} typedef ClientClientMessage;

struct ServerClientMessage
{
    long type;
    key_t connectToClientKey;
} typedef ServerMessage;

struct Client
{
    key_t key;
    long clientId;
    int available;
} typedef Client;
