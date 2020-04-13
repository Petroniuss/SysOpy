#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CLIENT_STOP 1
#define CLIENT_DISCONNECT 2
#define CLIENT_LIST 3
#define CLIENT_CONNECT 4
#define CLIENT_INIT 5
#define CLIENT_TO_CLIENT 6

#define SERVER_CHAT_INIT 1
#define SERVER_RESPONSE 2
#define SERVER_TERMINATED 3

#define SERVER_MAX_CLIENTS_CAPACITY 32
#define MAX_MSG_LENGTH 128

#define KEY_GENERATOR_PATH (getenv("HOME"))
#define PROJECT_IDENTIFIER 'P'

#define UNIQUE_KEY (ftok(KEY_GENERATOR_PATH, PROJECT_IDENTIFIER))
#define SERVER_KEY ((key_t)112358)

#define DELETE_QUEUE(id) (msgctl(id, IPC_RMID, NULL))
#define CREATE_QUEUE(key) (msgget(key, 0666 | IPC_PRIVATE))
#define GET_QUEUE(key) (msgget(key, 0666))

// TODO define custom messages and figure out a way to deserialize them.
// So it looks like i am allowed to have only one type for a message.
struct ClientMessage
{
    long type;
    long clientId;
    long clientKey;
    long connectToClientId; // another client id
    char msg[MAX_MSG_LENGTH];
} typedef ClientMessage;

struct ServerMessage
{
    long type;
    long clientId;
    long connectToClientId;
} typedef ServerMessage;

struct Client
{
    key_t key;
    long clientId;
    int available;
} typedef Client;
