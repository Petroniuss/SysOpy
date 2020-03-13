#include "string_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <sys/times.h>

// LOGGING ----------------------------------------------------------

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(0);
}

void logInfo(FILE* logFile, char* msg) {
    printf("%s\n", msg);
    fprintf(logFile, "%s\n", msg);
}

double timeDifference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void formatTime(char* buffer, clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    double real = timeDifference(start, end);
    double user = timeDifference(t_start -> tms_utime, t_end -> tms_utime);
    double system = timeDifference(t_start -> tms_stime, t_end -> tms_stime);
    
    sprintf(buffer, "Timing ---------------------------\n\tREAL_TIME: %fs\n\tUSER_TIME: %fs\n\tSYSTEM_TIME: %fs\n----------------------------------\n", real, user, system);
}

// -----------------------------------------------------------------

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

// Q-SORT-SYSTEM -----------------------------------------------

void swapLinesWithFileDescriptor(int fd, int i, int j, size_t bytes) {
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

        if (strcmp(e, pivot) <= 0) {
            swapLinesWithFileDescriptor(fd, ++smaller, i, bytes);
        }
    }

    free(pivot);
    free(e);

    swapLinesWithFileDescriptor(fd, ++smaller, i, bytes);

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

    if (fd == -1) {
        error("File does not exist");
    }

    size_t bytes = recordSize + 1;
    int from = 0;
    int to   = recordsNum - 1;

    qSortSysRecursive(fd, from, to, bytes);

    close(fd);
}

// -------------------------------------------------------------------

// Q-SORT-LIBRARY----------------------------------------------------

void swapLines(FILE* file, int i, int j, size_t bytes) {
    char* bufferI = malloc(sizeof(char) * (bytes + 1));
    char* bufferJ = malloc(sizeof(char) * (bytes + 1));

    long int iOff = i * bytes;
    long int jOff = j * bytes;

    fseek(file, iOff, 0);
    fread(bufferI, sizeof(char), bytes, file);

    fseek(file, jOff, 0);
    fread(bufferJ, sizeof(char), bytes, file);

    fseek(file, iOff, 0);
    fwrite(bufferJ, sizeof(char), bytes, file);
    fseek(file, jOff, 0);
    fwrite(bufferI, sizeof(char), bytes, file);

    free(bufferI);
    free(bufferJ);
}


int qSortLibPartition(FILE* file, int from, int to, size_t bytes) {
    char* pivot = malloc(sizeof(char) * (bytes + 1));
    char* e     = malloc(sizeof(char) * (bytes + 1));

    fseek(file, to * bytes, 0);
    fread(pivot, sizeof(char), bytes, file);

    int smaller = from - 1;
    int i       = from - 1;
    while (++i < to) {
        fseek(file, i * bytes, 0);
        fread(e, sizeof(char), bytes, file);

        if (strcmp(e, pivot) <= 0) {
            swapLines(file, ++smaller, i, bytes);
        }
    }

    free(pivot);
    free(e);

    swapLines(file, ++smaller, i, bytes);

    return smaller;
}

void qSortLibRecursive(FILE* file, int from, int to, size_t bytes) {
    if (from >= to) {
        return;
    }

    int mid = qSortLibPartition(file, from, to, bytes);

    qSortLibRecursive(file, from, mid - 1, bytes);
    qSortLibRecursive(file, mid + 1, to, bytes);
}


void qSortLib(char* filename, int recordsNum, int recordSize) {
    FILE* file = fopen(filename, "r+");

    if (!file) {
        error("File does not exist!");
    }

    size_t bytes = recordSize + 1;
    int from = 0;
    int to   = recordsNum - 1;

    qSortLibRecursive(file, from, to, bytes);

    fclose(file);
}


// -------------------------------------------------------------------


int main(int argc, char* argv[]) {
    FILE* logFile = fopen("log.txt", "a");
    
    clock_t startTime;
    clock_t endTime;
    struct tms* tmsStart = calloc(1, sizeof(struct tms*));
    struct tms* tmsEnd   = calloc(1, sizeof(struct tms*));

    char msg[250];
    int i = 0;
    srand(time(0));

    while (++i < argc) {
        const char* command = argv[i];
        startTime = times(tmsStart);
        
        if (strcmp("generate", command) == 0) {
            char* filename   = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            sprintf(msg, "Command: generate file \"%s\" with %s records, %s bytes each.", argv[i - 2], argv[i - 1], argv[i]);
            logInfo(logFile, msg);
            generate(filename, recordsNum, recordSize);
        } else if (strcmp("copy", command) == 0) { 
            char* src   = argv[++i];
            char* dest  = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            i += 1;

            sprintf(msg, "Command: copy src: \"%s\" to \"%s\", %s records, %s bytes each. Flag: %s", argv[i - 4], argv[i - 3], argv[i - 2], argv[i - 1], argv[i]);
            logInfo(logFile, msg);

            if (strcmp("-sys", argv[i]) == 0) {
                copySys(src, dest, recordsNum, recordSize);
            } else if (strcmp("-lib", argv[i]) == 0) {
                copyLib(src, dest, recordsNum, recordSize);
            } else {
                error("Not implemented");
            }

        } else if (strcmp("sort", command) == 0) {
            char* filename   = argv[++i];
            int   recordsNum = atoi(argv[++i]);
            int   recordSize = atoi(argv[++i]);

            i += 1;
            sprintf(msg, "Command: sort file: \"%s\" with %s records, %s bytes each. Flag: %s", argv[i - 3], argv[i - 2], argv[i - 1], argv[i]);
            logInfo(logFile, msg);

            if (strcmp("-sys", argv[i]) == 0) {
                qSortSys(filename, recordsNum, recordSize);
            } else if (strcmp("-lib", argv[i]) == 0) {
                qSortLib(filename, recordsNum, recordSize);
            } else {
                error("Not implemented");
            }

        } else {
            error("Failed to parse command");
        }

        endTime = times(tmsEnd);
        formatTime(msg, startTime, endTime, tmsStart, tmsEnd);
        logInfo(logFile, msg);
    }

    fclose(logFile);

    return 0;
}


