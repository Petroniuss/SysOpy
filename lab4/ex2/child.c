#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define SIGNAL SIGUSR1

#define FLAG_IGNORE  (1 << 1)
#define FLAG_HANDLE  (1 << 2)
#define FLAG_MASK    (1 << 3)
#define FLAG_PENDING (1 << 4)

void checkPending(int isParent) {
    sigset_t set;
    sigpending(&set);

    char* who = isParent ? "Parent" : "Child";

    if (sigismember(&set, SIGNAL)) {
        printf("%s sees pending signal - SIGUSR1 - %d.\n", who, SIGNAL);
    } else {
        printf("%s does not see pending signal.\n", who);
    }
} 

int main(int argc, char* argv[]) { 
    int option = atoi(argv[1]);

    if (option & FLAG_PENDING) {
        checkPending(0);
        return 0;
    }

    raise(SIGNAL);

    if (option & FLAG_IGNORE) {
        // Signal is ignored ...
        printf("Signal is ignored in child process\n");
    } else if (option & FLAG_HANDLE) {
        // Child process doesn't copy parent's handlers so 
        // the process terminates as soon as it receives SIGNAL.
    } else if (option & FLAG_MASK) {
        // Signal is blocked... so the process does not terminate.
        printf("Signal is blocked - child.\n");
    } else if (option & FLAG_PENDING) {
        checkPending(0);
    }

    return 0;
}