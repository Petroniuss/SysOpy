#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define SIGNAL SIGUSR1

#define FLAG_IGNORE  (1 << 1)
#define FLAG_HANDLE  (1 << 2)
#define FLAG_MASK    (1 << 3)
#define FLAG_PENDING (1 << 4)

int option = 0;

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(-1);
}
 
void setIgnore() {
    signal(SIGNAL, SIG_IGN);
    option |= FLAG_IGNORE;
}

void handle(int signal) {
    printf("Received signal - SIGUSR1 - %d\n", signal);
}

void setHandler() {
    signal(SIGNAL, handle);
    option |= FLAG_HANDLE;
}

void anotherHandle(int sig) {
    printf("What up\n");
}

void anotherHandler() {
    signal(SIGNAL, anotherHandle);
}

void setMask() {
    sigset_t oldMask;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGNAL);

    sigprocmask(SIG_SETMASK, &mask, &oldMask);
    option |= FLAG_MASK;
}

void setPending() {
    setMask();
    option ^= FLAG_MASK;
    option |= FLAG_PENDING;
}

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
    if (argc < 3) 
        error("Not enaugh arguments");

    char* flag = argv[1];
    int exec   = 0;

    if (strcmp("-ignore", flag) == 0) {
        setIgnore();
    } else if (strcmp("-handler", flag) == 0) {
        setHandler();
    } else if (strcmp("-mask", flag) == 0) {
        setMask();
    } else if (strcmp("-pending", flag) == 0) {
        setPending();
    } 

    if (strcmp("-exec", argv[2]) == 0) {
        exec = 1;
    } else if (strcmp("-fork", argv[2]) == 0) {
        exec = 0;
    } else {
        error("How to execute ?? -fork or -exec");
    }

    raise(SIGNAL);
    
    if (option & FLAG_IGNORE) {
        // Signal is ignored...
        printf("Signal is ignored in parent process\n");
    } else if (option & FLAG_HANDLE) {
        // Parent handles signal...
    } else if (option & FLAG_MASK) {
        // Signal is blocked...
        printf("Signal is blocked - parent.\n");
    } else if (option & FLAG_PENDING) {
        // Check if signal is pending
        checkPending(1);
    }

    int pid = fork();
    if (pid == 0) {
        if (exec) {
            char num [12];
            sprintf(num, "%d", option);
            
            execl("./child", "./child", num, NULL);
        }

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
    
    if (exec) {
        wait(NULL);
    }

    return 0;
}

// Bullet points
// Todo - Child process doesn't copy parent's handlers ?? (Sometimes it does but here it looks like it doesn't!) <-- Unexpected feature.
// Parent's pending signal is not visible in child (descendant)
// Masks are copied to child. 