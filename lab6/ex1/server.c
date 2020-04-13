#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "shared.h"

Client *clients[SERVER_MAX_CLIENTS_CAPACITY];

int serverIdentifier = -1;

void handleExit()
{
    printf("Server exits...");
    DELETE_QUEUE(serverIdentifier);
}

void handleStop(ClientMessage *msg)
{
}

int main(int argc, char *argv[])
{
    serverIdentifier = CREATE_QUEUE(SERVER_KEY);

    return 0;
}