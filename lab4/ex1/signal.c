#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int waiting = 0;

void onReceiveINT(int signum) {
    printf("\nReceived signal SIGINT - %d\n\tFinishing execution.\n", signum);
    exit(0);
}

void onReceiveSTP(int signum) {
    if (!waiting) 
        printf("\nWaiting for \n\tCTRL+Z - continue executing\n\tCTRL+C - terminate\n");
    else 
        printf("\n");
    
    waiting = 1 - waiting;
}


int main(int argc, char* argv[]) {
    signal(SIGINT, onReceiveINT);

    struct sigaction act;

    act.sa_handler = onReceiveSTP;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTSTP, &act, NULL);

    while (1) {
        if (waiting) 
            pause();
        printf("Printing current directory \n");
        system("ls .");
        printf("Sleeping for 2 seconds..\n");
        sleep(2);
    }

    return 0;
}

























