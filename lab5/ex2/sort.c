#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#define BUFFER_SIZE 50

void error(const char* msg) {
    printf("Error: %s\n", msg);
    
    exit(EXIT_FAILURE);
}  

char* concat(const char* s1, const char* s2) {
    int   len    = strlen(s1) + strlen(s2) + 1;
    char* result = (char*) malloc(len);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

int main(int argc, char* argv[]) { 
    if (argc < 2) 
        error("Not enaugh arguments!");

    char* filename = argv[1];
    char* command  = concat("sort ", filename);
    FILE* pipe = popen(command, "r");
    char buffer [BUFFER_SIZE];

    for (int read = fread(buffer, sizeof(char), BUFFER_SIZE, pipe);
             read != 0;
             read = fread(buffer, sizeof(char), BUFFER_SIZE, pipe)) {
        
        buffer[read] = '\0';
        printf("%s", buffer);
    }

    return EXIT_SUCCESS;
}