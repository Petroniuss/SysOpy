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
#include <fcntl.h>

#include "utils_lib.h"
#include "matrix_lib.h"
// We need to synchronize on file!

// Todo - fix this damn flock..
void runWorker(Matrix* matrixA, Matrix* matrixB, Matrix* matrixX, int maxTime, int colStart, int colEnd) {
    int n = matrixA -> cols;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    // This way we cannot acquire lock.
    // matrixX -> filePtr = fopen(resultFilename, "w+");

    // int fd = fileno(matrixX -> filePtr);

    int finishedMultiplications = 0;
    struct timespec start = nowRealTime();

    int rowCounter = 0;
    int colCounter = colStart;
    
    readRow(matrixA, row, 0);
    readColumn(matrixB, col, colStart);

    while(colCounter < colEnd && realTime(start) < maxTime) {
        int result = dotVectors(row, col, n);
        
        // Writing result -- error prone task.
        flock(fileno(matrixX -> filePtr), LOCK_EX);
        printf("Locked %d\n", colStart);
        writeResult(matrixX, rowCounter, colCounter, result);
        printf("Written result %d\n", colStart);
        flock(fileno(matrixX -> filePtr), LOCK_UN);
        printf("UnLocked %d\n", colStart);        

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

    // printf("PID: %d, Finished time: %fs\n", getpid() ,realTime(start));

    exit(finishedMultiplications);
}

int main(int argc, char* argv[]) {
    char* filenameA = argv[1];
    char* filenameB  = argv[2];
    char* resultFilename = argv[3];

    int colStart = atoi(argv[4]);
    int colEnd   = atoi(argv[5]);
    int maxTime  = atoi(argv[6]);

    Matrix* matrixA = initMatrix(filenameA);
    Matrix* matrixB = initMatrix(filenameB);
    Matrix* matrixX = malloc(sizeof(Matrix));

    matrixX -> rows = matrixA -> rows;
    matrixX -> cols = matrixB -> cols;
    matrixX -> filePtr = fopen(resultFilename, "r+");

    printf("\n%s %s %s %d %d %d\n", filenameA, filenameB, resultFilename, colStart, colEnd, maxTime);
    runWorker(matrixA, matrixB, matrixX, maxTime, colStart, colEnd);
    
    return 1;
}