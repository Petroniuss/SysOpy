#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>  

#include "utils_lib.h"

void error(const char* msg) {
    printf("Error: %s\n", msg);
    
    exit(EXIT_FAILURE);
}

// Solution with exec/fork/pipe.
void runProcess(char* line) {
    int tokenC;
    char** tokens = breakLineBySpaces(line, &tokenC);

    if (tokenC < 1)
        error("Each line must be a valid command!");

    // Let's calculate how many pipes we need!
    int pipesC = 0;
    for (int i = 0; i < tokenC; i++) {
        char* token = tokens[i];
        if (strcmp("|", token) == 0) 
            pipesC += 1;
    }

    // Initialize pipes!
    int** pipes = malloc(sizeof(int*) * (pipesC + 2));
    for (int i = 0; i < pipesC + 2; i++) {
        pipes[i] = malloc(sizeof(int) * 2);
    }

    int isFirst = 1;
    int prevPipe[2];
    int nextPipe[2];    

    // It's essential to close unused file descriptors!
    int startI = 0;
    for (int i = 0; i < tokenC; i++) {
        char* token = tokens[i];
        if (strcmp("|", token) == 0) { 
            pipe(nextPipe);
            if (fork() == 0) {
                char** args = (char**) subarrayWithNull((void**) tokens, startI, i);
                char*  programName = args[0];
                
                if (isFirst) {
                    dup2(nextPipe[1], 1);
                    close(nextPipe[0]);
                    isFirst = 0;
                } else {
                    dup2(prevPipe[0], 0);
                    dup2(nextPipe[1], 1);

                    close(prevPipe[1]);
                    close(nextPipe[0]);
                }

                execvp(programName, args);
                exit(1);
            } else {
                if (isFirst) {
                    isFirst = 0;
                } else {
                    close(prevPipe[0]);
                    close(prevPipe[1]);
                }

                prevPipe[0] = nextPipe[0];
                prevPipe[1] = nextPipe[1];
                startI = i + 1;
            }
        }
    }   


    dup2(prevPipe[0], 0);
    close(prevPipe[1]);
    char** args = (char**) subarrayWithNull((void**) tokens, startI, tokenC);
    char*  programName = args[0];
    execvp(programName, args);

    exit(1);
}

int main(int argc, char* argv[]) { 
    if (argc < 2) 
        error("Not enaugh arguments!");
    
    char* filename = argv[1];   
    int    linesC = countLines(filename);
    char** linesV = readLines(linesC, filename);

    for (int i = 0; i < linesC; i++) {
        if (fork() == 0) 
            runProcess(linesV[i]);
    }
    
    printf("Main\n  Waiting...\n");
    for (int i = 0; i < linesC; i++) 
        wait(NULL);

    printf("Main\n  Done...\n");

    return 0;
}
