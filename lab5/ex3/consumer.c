#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>  
#include "utils_lib.h"

#define SLEEP_TIME 1

void error(const char* msg) {
    printf("Error: %s\n", msg);
    
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) { 
    if (argc < 4) 
        error("Not enaugh arguments!");

    char* pipeFilename = argv[1];
    char* filename = argv[2];
    int N = atoi(argv[3]);

    FILE* file = fopen(filename, "w");
    FILE* pipe = fopen(pipeFilename, "r");

    char buffer [N + 1];

    for (int read = fread(buffer, sizeof(char), N, pipe);
             read != 0;
             read = fread(buffer, sizeof(char), N, pipe)) {
        
        buffer[read] = '\0';
        fprintf(file, "%s", buffer);
    }

    return 0;
}