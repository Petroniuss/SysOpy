#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define FLAG_SIGQUE  (1 << 1)
#define FLAG_SIGRT   (1 << 2)
#define FLAG_KILL    (1 << 3)

int flag = 0;
int catcherPid;
int nSignals;

int receivedSIGUSR2 = 0;
int received        = 0;

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

void handleSIGUSR1(int signal) {
    received++;
}

void handleSIGUSR2(int signal) {
    receivedSIGUSR2 = 1;
}

void execKill() {
    // Install handlers
    signal(SIGUSR1, handleSIGUSR1);
    signal(SIGUSR2, handleSIGUSR2);

    // Block other signals
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    // Send signals
    for (int i = 0; i < nSignals; i++) {
        kill(catcherPid, SIGUSR1);
    }

    kill(catcherPid, SIGUSR2);
    
    while (!receivedSIGUSR2) {
        pause();
    }
}
 

int main(int argc, char* argv[]) { 
    if (argc < 3) 
        error("Not enaugh arguments"); 

    catcherPid = atoi(argv[1]);
    nSignals   = atoi(argv[2]);

    char* execFlag = argv[3];

    if (strcmp("-kill", execFlag) == 0) 
        flag |= FLAG_KILL;
    else if (strcmp("-sigqueue", execFlag) == 0)
        flag |= FLAG_SIGRT;
    else if (strcmp("-sigrt", execFlag) == 0)
        flag |= FLAG_SIGQUE;
    else
        error("Enter valid flag");

    
    if (FLAG_KILL & flag) 
        execKill();


    return 0;
}