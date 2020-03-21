#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
#include <linux/limits.h>
#include <time.h>
#include "utils_lib.h"
#include "matrix_lib.h"

void error(char* msg) {
    printf("Error: %s \n", msg);
    exit(0);
}

Matrix* initMatrix(char filename[PATH_MAX]) {
    Matrix* matrix = malloc(sizeof(Matrix));    
    matrix -> filePtr = fopen(filename, "r");

    if (!matrix -> filePtr) {
        error("Cannot open matrix file");
    }

    matrix -> rows    = countLines(matrix -> filePtr);
    matrix -> cols    = countElemsInFirstRow(matrix -> filePtr);

    return matrix;
}


int main(int argc, char* argv[]) {
    if (argc < 4) 
        error("Not enaugh arguments");
    

    char* configFile = argv[1];
    int workersNum   = atoi(argv[2]);
    int timeLimit    = atoi(argv[3]);
    
    char filenameA[PATH_MAX]; 
    char filenameB[PATH_MAX];
    char resultFilename[PATH_MAX];

    FILE* configFilePtr = fopen(configFile, "r");

    if (!configFilePtr)
        error("Not found configuration file.");

    if(fscanf(configFilePtr, "%s %s %s", filenameA, filenameB, resultFilename) != 3)
        error("Supplied configurtion file is invalid.");

    Matrix* matrixA = initMatrix(filenameA);
    Matrix* matrixB = initMatrix(filenameB);

    if (matrixA -> cols != matrixB -> rows) {
        error("Number of columns in matrixA must be equal to number of columns in matrixB");
    }

    if (workersNum > matrixB -> cols || workersNum <= 0) {
        error("Invalid number of worker processes");
    }

    
    Matrix* matrixX = createResultFile(resultFilename, matrixA -> rows, matrixB -> cols);

    int* workersPids  = malloc(sizeof(int) * workersNum);
    double runningSum = 0.0;
    double factor     = ((double)  matrixB -> cols ) / workersNum;

    for (int i = 0; i < workersNum; i++) {
        int start = (int) runningSum;
        runningSum += factor;
        int end   = (int) runningSum;

        int forked = fork();
        if (forked == 0) { // child
            // Should probably do some stuff..
            if (i == workersNum - 1) {
                end = matrixB -> cols;
            }
            // printf("Worker: %d, start: %d, end: %d \n", i, start, end);
            runWorker(matrixA, matrixB, matrixX, timeLimit, start, end);
        } else { // parent
            workersPids[i] = forked;    
        }
    }

    for (int i = workersNum - 1; i >= 0; i--) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    return 0;
}

// Change the way you measure time.
float timeDifference(clock_t t1, clock_t t2){
    return ((float)(t2 - t1) / sysconf(CLOCKS_PER_SEC));
}

float runningTime(clock_t startTime) {
    clock_t end = clock();
    float seconds = timeDifference(startTime, end);
    
    return seconds;
}

void runWorker(Matrix* matrixA, Matrix* matrixB, Matrix* matrixX, int maxTime, int colStart, int colEnd) {
    int n = matrixA -> cols;
    int m = matrixA -> rows;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    int finishedMultiplications = 0;
    clock_t start = clock();

    int rowCounter = 0;
    int colCounter = colStart;
    
    readRow(matrixA, row, 0);
    readColumn(matrixB, col, colStart);

    while(colCounter < colEnd && runningTime(start) < maxTime) {
        int result = dotVectors(row, col, n);
        printf("Row:%d dot column:%d = %d, time: %f\n", rowCounter, colCounter, result, runningTime(start));
        
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

    exit(finishedMultiplications);
}
