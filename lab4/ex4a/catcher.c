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

int received = 0;
int receivedTerminalSignal = 0;
int senderPID;

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

// SIGQUEUE

void handle_queue_SIGUSR1(int sig, siginfo_t* info, void* ucontext) {
    received++;
    printf("Recevied \n");
}

void handle_queue_SIGUSR2(int sig, siginfo_t* info, void* ucontext) {
    receivedTerminalSignal = 1;
    printf("Received terminating\n");
}

void execSIGQUEUE() {
    // Install handlers
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

    printf("Catcher\n\tPid - %d.\n", getpid());

    while (!receivedTerminalSignal) {
        pause();
    }

    for (int i = 0; i < received; i++) {
        union sigval val;
        val.sival_int = i + 1;
        sigqueue(senderPID, SIGUSR1, val);
    }

    union sigval val;
    val.sival_int = received;
    sigqueue(senderPID, SIGUSR2, val);

    printf("Catcher\n\tReceived %d signals.\n", received);
}

// --

// SIGRT
void handleSIGRT1(int signal) {
    received++;
}

void handleSIGRT2(int sig, siginfo_t* info, void* ucontext) {
    senderPID = info -> si_pid;
    receivedTerminalSignal = 1;
}

void execSIGRT() {
    // Install handlers
    struct sigaction act1;
    act1.sa_handler = handleSIGRT1;
    act1.sa_flags = 0;
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

    printf("Catcher\n\tPid - %d.\n", getpid());

    while (!receivedTerminalSignal) {
        sigsuspend(&mask);
    }

    for (int i = 0; i < received; i++) {
        kill(senderPID, SIGRTMIN + 1);
    }

    kill(senderPID, SIGRTMIN + 2);

    printf("Catcher\n\tReceived %d signals.\n", received);
}

// SIGQUEUE

void handleSIGUSR1_SIGQUEUE(int signal) {
    received++;
}

void handleSIGUSR2_SIGQUEUE(int sig, siginfo_t* info, void* ucontext) {
    senderPID = info -> si_pid;
    receivedTerminalSignal = 1;
}

// --

// KILL ---------------------------- 

void handleSIGUSR1(int signal) {
    received++;
}

void handleSIGUSR2(int sig, siginfo_t* info, void* ucontext) {
    senderPID = info -> si_pid;
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
    act.sa_sigaction = handleSIGUSR2;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGUSR2, &act, NULL);
    
    // Block other signals
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    printf("Catcher\n\tPid - %d.\n", getpid());

    while (!receivedTerminalSignal) {
        pause();
    }

    for (int i = 0; i < received; i++) {
        kill(senderPID, SIGUSR1);
    }

    kill(senderPID, SIGUSR2);

    printf("Catcher\n\tReceived %d signals.\n", received);
}

// ------------------------------------------------

int main(int argc, char* argv[]) { 
    char* execFlag = argv[1];

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