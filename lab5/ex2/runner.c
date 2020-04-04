#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include<sys/wait.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    char* files [5] = { "test/sh1.txt",
                        "test/sh2.txt",
                        "test/sh3.txt",
                        "test/sh4.txt",
                        "test/sh5.txt"};
                        
    char* outputFile = "test/sh-out.txt";
    char* namedPipeFilename = "test/sh-pipe";

    mkfifo(namedPipeFilename, 0666);

    // Create 5 producers
    for (int i = 0; i < 5; i++) {
        if (fork() == 0) {
            int n = rand() % 10 + 3;
            char buff [3];
            sprintf(buff, "%d", n);

            char* args [5] = {"./producer", namedPipeFilename, files[i], buff, NULL};
            execvp("./producer", args);
        }
    }  

    // Run consumer
    if (fork() == 0) {
        int n = rand() % 10 + 3;
        char buff [3];
        sprintf(buff, "%d", n);

        char* args [5] = {"./consumer", namedPipeFilename, outputFile, buff, NULL};
        execvp("./consumer", args); 
    }

    for (int i = 0; i < 6; i++) {
        wait(NULL);
    }

    return 0;
}