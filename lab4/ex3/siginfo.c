#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Arithemtic exception
#define SIG_1 SIGFPE
// Sent when child process finishes execution
#define SIG_2 SIGCHLD
// Sent when IO operation occurs 
#define SIG_3 SIGSEGV


void arithmeticExceptionHandler(int sig, siginfo_t* info, void* ucontext) {
    // si_code indicates why the signal has been sent. 
    // In this case it gives us information about the error that occured.
    if (info -> si_code == FPE_INTDIV) {
        printf("Integer division by zero occured\n\t si_code - %d\n", FPE_INTDIV);
    }

    exit(1);
}

void childSignalHanlder(int sig, siginfo_t* info, void* ucontext) {
    printf("Child has finished execution\n");
    printf("\tReturn code - %d\n", info -> si_status);    
}

void segFaultHandler(int sig, siginfo_t* info, void* ucontext) {
    printf("Segmentation fault occured.\n");
    // si_addr gives us the faulty address of instruction
    // In our case it's 5 since we tried to access 5th element of empty array.
    printf("\tFaulty address: %p\n", info->si_addr);

    exit(0);
}


int main(int argc, char* argv[]) {
    // We install arithmetic exception handler.
    struct sigaction act;
    act.sa_sigaction = arithmeticExceptionHandler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIG_1, &act, NULL);

    // We install child signal handler
    struct sigaction childAct;
    childAct.sa_sigaction = childSignalHanlder;
    childAct.sa_flags = SA_SIGINFO;
    sigemptyset(&childAct.sa_mask);
    sigaction(SIG_2, &childAct, NULL);

    // We install handler for io operations
    struct sigaction segAct;
    segAct.sa_sigaction = segFaultHandler;
    segAct.sa_flags = SA_SIGINFO;
    sigemptyset(&segAct.sa_mask);
    sigaction(SIG_3, &segAct, NULL);

    // Here we're provoking various errors.
    int forked = fork();
    if (forked == 0) {
        int zero = 0;
        int foo = 2 / zero;
        printf("I managed to divide by zero! ==> %d\n", foo);
    }

    // Wait for child signal
    sleep(10);
    char* foo = NULL;
    foo[5] = 'c';
    printf("Escaped segFault %s...\n", foo);
    
    return 0;
}