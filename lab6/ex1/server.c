#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "shared.h"

int serverIdentifier = -1;

int main(int argc, char *argv[])
{
    serverIdentifier = CREATE_QUEUE(SERVER_KEY);

    DELETE_QUEUE(serverIdentifier);

    return 0;
}