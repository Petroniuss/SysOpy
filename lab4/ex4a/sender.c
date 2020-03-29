#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define FLAG_SIGQUE  (1 << 1)
#define FLAG_SIGRT   (1 << 2)
#define FLAG_KILL    (1 << 3)

int flag = 0;
int catcherPid;
int nSignals;

int receivedTerminalSignal = 0;
int received        = 0;

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

void handle_queue_SIGUSR1(int sig, siginfo_t* info, void* ucontext) {
    printf("Sender\n\tNext signal number should be %d.\n", info -> si_int);
    received++;
}

void handle_queue_SIGUSR2(int sig, siginfo_t* info, void* ucontext) {
    printf("Sender\n\tI know catcher received %d signals.\n", info -> si_int);
    receivedTerminalSignal = 1;
}


// MY SYSTEM DOES NOT IMPLEMENT SIGQUEUE.
void execSIGQUEUE() {
    struct sigaction act1;
    act1.sa_sigaction = handle_queue_SIGUSR1;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaddset(&act1.sa_mask, SIGUSR2);
    sigaction(SIGUSR1, &act1, NULL);

    struct sigaction act;
    act.sa_sigaction = handle_queue_SIGUSR2;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGUSR2, &act, NULL);  

    // Block other signals
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGINT);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    union sigval val;
    val.sival_int = 0;

    // Send signals
    printf("Sending\n");
    for (int i = 0; i < nSignals; i++) {
        sigqueue(catcherPid, SIGUSR1, val);
    }
    int ret = sigqueue(catcherPid, SIGUSR2, val);
    printf("Sent to %d, %d, %d, %d\n", catcherPid, SIGUSR2, ret, errno);

    while (!receivedTerminalSignal) {
        pause();
    }

    printf("Sender\n\tReceived %d signals.\n\tHe should have received %d.\n", received, nSignals);
}

// SIGRT

void handleSIGRT1(int sig, siginfo_t* info, void* ucontext) {
    received++;
}

void handleSIGRT2(int sig, siginfo_t* info, void* ucontext) {
    receivedTerminalSignal = 1;
}

void execSIGRT() {
    // // Install handlers
    struct sigaction act1;
    act1.sa_sigaction = handleSIGRT1;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaddset(&act1.sa_mask, SIGRTMIN + 2);
    sigaction(SIGRTMIN + 1, &act1, NULL);

    struct sigaction act;
    act.sa_sigaction = handleSIGRT2;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGRTMIN + 1);
    sigaction(SIGRTMIN + 2, &act, NULL);  

    // Block other signals
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGRTMIN + 1);
    sigdelset(&mask, SIGRTMIN + 2);
    sigdelset(&mask, SIGQUIT);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    // Send signals
    for (int i = 0; i < nSignals; i++) {
        kill(catcherPid, SIGRTMIN + 1);
        sleep(1);
    }

    kill(catcherPid, SIGRTMIN + 2);

    while (!receivedTerminalSignal) {
        sigsuspend(&mask);
    }

    printf("Sender\n\tReceived %d signals.\n\tHe should have received %d.\n", received, nSignals);
}

// 

// KILL

void handleSIGUSR1(int signal) {
    received++;
}

void handleSIGUSR2(int signal) {
    receivedTerminalSignal = 1;
}

void execKill() {
    // Install handlers
    struct sigaction act1;
    act1.sa_handler = handleSIGUSR1;
    act1.sa_flags = 0;
    sigemptyset(&act1.sa_mask);
    sigaddset(&act1.sa_mask, SIGUSR2);
    sigaction(SIGUSR1, &act1, NULL);

    struct sigaction act;
    act.sa_handler = handleSIGUSR2;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGUSR2, &act, NULL);

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
    
    while (!receivedTerminalSignal) {
        pause();
    }

    printf("Sender\n\tReceived %d signals.\n\tHe should have received %d.\n", received, nSignals);
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
        flag |= FLAG_SIGQUE;
    else if (strcmp("-sigrt", execFlag) == 0)
        flag |= FLAG_SIGRT;
    else
        error("Enter valid flag");

    
    if (FLAG_KILL & flag) 
        execKill();
    else if (FLAG_SIGQUE & flag)
        execSIGQUEUE();
    else if (FLAG_SIGRT & flag)
        execSIGRT();


    return 0;
}