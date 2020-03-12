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

    free(buffer);

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

    free(buffer);

    fclose(srcFilePtr);
    fclose(destFilePtr);
}

void swapLines(int fd, int i, int j, size_t bytes) {
    char* bufferI = malloc(sizeof(char) * (bytes + 1));
    char* bufferJ = malloc(sizeof(char) * (bytes + 1));

    off_t iOff = i * bytes;
    off_t jOff = j * bytes;

    lseek(fd, iOff, SEEK_SET);
    read(fd, bufferI, bytes);
    lseek(fd, jOff, SEEK_SET);
    read(fd, bufferJ, bytes);

    lseek(fd, iOff, SEEK_SET);
    write(fd, bufferJ, bytes);
    lseek(fd, jOff, SEEK_SET);
    write(fd, bufferI, bytes);

    free(bufferI);
    free(bufferJ);
}

int qSortSysPartition(int fd, int from, int to, size_t bytes) {
    char* pivot = malloc(sizeof(char) * (bytes + 1));
    char* e     = malloc(sizeof(char) * (bytes + 1));

    lseek(fd, to * bytes, SEEK_SET);
    read(fd, pivot, bytes);

    int smaller = from - 1;
    int i       = from - 1;
    while (++i < to) {
        lseek(fd, i * bytes, SEEK_SET);
        read(fd, e, bytes);

        if (strcmp(e, pivot) < 0) {
            swapLines(fd, ++smaller, i, bytes);
        }
    }

    swapLines(fd, ++smaller, i, bytes);

    return smaller;
}



void qSortSysRecursive(int fd, int from, int to, size_t bytes) {
    if (from >= to) {
        return;
    }

    int mid = qSortSysPartition(fd, from, to, bytes);

    qSortSysRecursive(fd, from, mid - 1, bytes);
    qSortSysRecursive(fd, mid + 1, to, bytes);
}


void qSortSys(char* filename, int recordsNum, int recordSize) {
    int fd = open(filename, O_RDWR);

    size_t bytes = recordSize + 1;
    int from = 0;
    int to   = recordsNum - 1;

    qSortSysRecursive(fd, from, to, bytes);

    close(fd);
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
        } else if (strcmp("sort", command) == 0) {
            char* filename   = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            i += 1;

            if (strcmp("-sys", argv[i]) == 0) {
                qSortSys(filename, recordsNum, recordSize);
            } else if (strcmp("-lib", argv[i]) == 0) {
                error("Not impl");
            } else {
                error("Not implemented");
            }

        } else {
            error("Failed to parse command");
        }

        printf("Done\n");
    }

    return 0;
}


