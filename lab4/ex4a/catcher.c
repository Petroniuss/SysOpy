#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define FLAG_SIGQUE  (1 << 1)
#define FLAG_SIGRT   (1 << 2)
#define FLAG_KILL    (1 << 3)

#define SIGRT1 SIGRTMIN
#define SIGRT2 SIGRTMIN + 1

int receivedTerminalSignal = 0;
int flag     = 0;
int received = 0;
int senderPID;

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

// -------------- SIGQUEUE --------------

void handle_queue_SIGUSR1(int sig) {
    received++;
}

void handle_queue_SIGUSR2(int sig, siginfo_t* info, void* ucontext) {
    senderPID = info -> si_pid;
    receivedTerminalSignal = 1;
}

void execSIGQUEUE() {
    // Install handlers
    struct sigaction act1;
    act1.sa_handler = handle_queue_SIGUSR1;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaddset(&act1.sa_mask, SIGUSR2);
    sigaction(SIGUSR1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction = handle_queue_SIGUSR2;
    act2.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaddset(&act2.sa_mask, SIGUSR1);
    sigaction(SIGUSR2, &act2, NULL);
    
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

    for (int i = 1; i <= received; i++) {
        union sigval val;
        val.sival_int = i + 1;
        sigqueue(senderPID, SIGUSR1, val);
    }

    union sigval val;
    val.sival_int = received;
    sigqueue(senderPID, SIGUSR2, val);

    printf("Catcher\n\tReceived %d signals.\n", received);
}
// ----------------------------------


//-------------- SIGRT --------------

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
    sigaddset(&act1.sa_mask, SIGRT2);
    sigaction(SIGRT1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction = handleSIGRT2;
    act2.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaddset(&act2.sa_mask, SIGRT1);
    sigaction(SIGRT2, &act2, NULL);
    
    // Block other signals
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGRT1);
    sigdelset(&mask, SIGRT2);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    printf("Catcher\n\tPid - %d.\n", getpid());

    while (!receivedTerminalSignal) {
        pause();
    }

    for (int i = 0; i < received; i++) {
        kill(senderPID, SIGRT1);
    }

    kill(senderPID, SIGRT2);

    printf("Catcher\n\tReceived %d signals.\n", received);
}
// ------------------------------------------------

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