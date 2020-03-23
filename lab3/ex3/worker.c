#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
#include <linux/limits.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h> 
#include <fcntl.h>

#include "utils_lib.h"
#include "matrix_lib.h"

void runWorker(Matrix* matrixA, Matrix* matrixB, Matrix* matrixX, int maxTime, int colStart, int colEnd) {
    int n = matrixA -> cols;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);
    int pid  = getpid();

    int finishedMultiplications = 0;
    struct timespec start = nowRealTime();

    int rowCounter = 0;
    int colCounter = colStart;
    
    readRow(matrixA, row, 0);
    readColumn(matrixB, col, colStart);

    while(colCounter < colEnd && realTime(start) < maxTime) {
        // Here we perfom logging
        logSystemUsage(RUSAGE_SELF, pid);
        int result = dotVectors(row, col, n);
        
        // Writing result -- error prone task.
        flock(fileno(matrixX -> filePtr), LOCK_EX);
        writeResult(matrixX, rowCounter, colCounter, result);
        flock(fileno(matrixX -> filePtr), LOCK_UN);

        finishedMultiplications += 1;

        rowCounter++;
        if (rowCounter == matrixA -> rows) { 
            rowCounter = 0;
            colCounter++;

            readRow(matrixA, row, 0);
            if (colCounter < colEnd) {
                readColumn(matrixB, col, colCounter);
            }
        } else {
            readNextRow(matrixA, row);
        }
    }

    fclose(matrixA -> filePtr);
    fclose(matrixB -> filePtr);
    fclose(matrixX -> filePtr);

    logSystemUsage(RUSAGE_SELF, pid);

    exit(finishedMultiplications);
}

int main(int argc, char* argv[]) {
    char* filenameA = argv[1];
    char* filenameB  = argv[2];
    char* resultFilename = argv[3];

    int colStart = atoi(argv[4]);
    int colEnd   = atoi(argv[5]);
    int maxTime  = atoi(argv[6]);
    int  cpuTimeLimit       = atoi(argv[7]);
    int  virtualMemoryLimit = atoi(argv[8]); 

    long int memoryLimitBytes = 1000000 * virtualMemoryLimit;

    // Set limits for cpu-time
    struct rlimit cpuRL;
    getrlimit (RLIMIT_CPU, &cpuRL);

    cpuRL.rlim_cur = cpuTimeLimit;
    setrlimit(RLIMIT_CPU, &cpuRL);

    // Set limits for virtual-memory-space
    struct rlimit memRL;
    getrlimit(RLIMIT_AS, &memRL);
    
    memRL.rlim_cur = memoryLimitBytes;
    setrlimit(RLIMIT_AS, &memRL);

    Matrix* matrixA = initMatrix(filenameA);
    Matrix* matrixB = initMatrix(filenameB);
    Matrix* matrixX = malloc(sizeof(Matrix));

    matrixX -> rows = matrixA -> rows;
    matrixX -> cols = matrixB -> cols;
    matrixX -> filePtr = fopen(resultFilename, "r+");

    // printf("\n%s %s %s %d %d %d\n", filenameA, filenameB, resultFilename, colStart, colEnd, maxTime);
    runWorker(matrixA, matrixB, matrixX, maxTime, colStart, colEnd);
    
    return 1;
}