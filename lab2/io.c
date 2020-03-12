#include "string_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(0);
}

void generate(char* filename, int recordsNum, int recordSize) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

    while (recordsNum-- > 0) {
        char* rand = randomString(recordSize);
        char* buffer = concat(rand, "\n");

        write(fd, buffer, recordSize + 1);

        free(rand);
        free(buffer);
    }

    close(fd);
}

void copySys(char* src, char* dest, int recordsNum, int recordSize) {
    int srcFd = open(src, O_RDONLY);

    if (srcFd == -1) {
        error("Source file does not exist");
    }

    int destFd = open(dest, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
    char* buffer = (char*) malloc(sizeof(char) * (recordSize + 2));
    size_t bytes = recordSize + 1;

    while (recordsNum-- > 0) {
        read(srcFd, buffer, bytes);
        write(destFd, buffer, bytes);
    }

    close(srcFd);
    close(destFd);
}

void copyLib(char* src, char* dest, int recordsNum, int recordSize) {
    FILE* srcFilePtr = fopen(src, "r");

    if (!srcFilePtr) {
        error("Source file does not exist");
    }

    FILE* destFilePtr = fopen(dest, "a+");

    char* buffer = (char*) malloc(sizeof(char) * (recordSize + 2));
    size_t bytes = recordSize + 1;

    while (recordsNum-- > 0) {
        fread(buffer, sizeof(char), bytes, srcFilePtr);
        fwrite(buffer, sizeof(char), bytes, destFilePtr);
    }

    fclose(srcFilePtr);
    fclose(destFilePtr);
}

int main(int argc, char* argv[]) {
    char msg[250] = "";
    int i = 0;
    while (++i < argc) {
        const char* command = argv[i];
        printf("%s\n", command);
        
        if (strcmp("generate", command) == 0) {
            char* filename   = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            sprintf(msg, "Command: generate file \"%s\" with %s records, %s bytes each. \n", argv[i - 2], argv[i - 1], argv[i]);
            printf("%s", msg);
            generate(filename, recordsNum, recordSize);
        } else if (strcmp("copy", command) == 0) { 
            char* src   = argv[++i];
            char* dest  = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            i += 1;

            if (strcmp("-sys", argv[i]) == 0) {
                copySys(src, dest, recordsNum, recordSize);
            } else if (strcmp("-lib", argv[i]) == 0) {
                copyLib(src, dest, recordsNum, recordSize);
            } else {
                error("Not implemented");
            }

            sprintf(msg, "Command: copy src: \"%s\" to \"%s\", %s records, %s bytes each. Flag: %s \n", argv[i - 4], argv[i - 3], argv[i - 2], argv[i - 1], argv[i]);
            printf("%s", msg);
        } else {
            error("Failed to parse command");
        }

        printf("Done\n");
    }

    return 0;
}


