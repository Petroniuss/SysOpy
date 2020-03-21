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

void error(char* msg) {
    printf("Error: %s \n", msg);
    exit(1);
}

void runSimpleWorker(Matrix* matrixA, Matrix* matrixB, int maxTime, int colStart, int colEnd, char* outputFile) {
    FILE* ptr = fopen(outputFile, "w");

    int n = matrixA -> cols;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    int finishedMultiplications = 0;
    struct timespec start = nowRealTime();

    int rowCounter = 0;
    int colCounter = colStart;
    
    readRow(matrixA, row, 0);
    readColumn(matrixB, col, colStart);

    while(rowCounter < matrixA -> rows && realTime(start) < maxTime) {
        int result = dotVectors(row, col, n);
        
        fprintf(ptr, "%d", result);

        finishedMultiplications += 1;
        
        colCounter++;
        if (colCounter == colEnd) {
            colCounter = colStart;
            rowCounter++;
            fputc('\n', ptr);

            if(rowCounter < matrixA -> rows)
                readNextRow(matrixA, row);
        } else {
            fputc(' ', ptr);
        }

        readColumn(matrixB, col, colCounter);
    }


    fclose(ptr);

    exit(finishedMultiplications);
}

void distinctFilesManager(Matrix* matrixA, Matrix* matrixB, char* resultFilename, int workersNum, int maxTime) {
    int* workersPids  = malloc(sizeof(int) * workersNum);
    char** files      = malloc(sizeof(char*) * (workersNum + 1));
    double runningSum = 0.0;
    double factor     = ((double)  matrixB -> cols ) / workersNum;

    for (int i = 0; i < workersNum; i++) {
        int start = (int) runningSum;
        runningSum += factor;
        int end   = (int) runningSum;
        files[i] = malloc(sizeof(char) * 150);
        sprintf(files[i], "test/worker-%d", start);
        
        int forked = fork();
        if (forked == 0) { // child
            if (i == workersNum - 1) 
                end = matrixB -> cols;
            
            runSimpleWorker(matrixA, matrixB, maxTime, start, end, files[i]);
        } else { 
            workersPids[i] = forked;    
        }
    }

    for (int i = workersNum - 1; i >= 0; i--) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    files[workersNum] = NULL;

    char**args = malloc(sizeof(char*) * (workersNum + 3));

    args[0] = "paste";
    args[1] = "-d ";
    for(int i = 2; i <= workersNum + 1; i++) {
        args[i] = files[i - 2];
    }
    args[workersNum + 2] = NULL;

    int vPid = vfork();
    if (vPid == 0) {
        int fd = open(resultFilename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fd, 1);
        close(fd);
        execv("/usr/bin/paste", args);
    }

    wait(NULL);

    for (int i = 0; i < workersNum; i++) {
        remove(files[i]);
    }

    exit(0);
}

// COMMON FILE WORKERS!

// Todo - fix this damn flock..
void runWorker(Matrix* matrixA, Matrix* matrixB, Matrix* matrixX, char* resultFilename, int maxTime, int colStart, int colEnd) {
    int n = matrixA -> cols;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    printf("Desc: %d \n", fileno(matrixX -> filePtr));
    fclose(matrixX -> filePtr); 
    // matrixX -> filePtr = fopen(resultFilename, "w+");
    int fd = open(resultFilename, O_RDWR);
    printf("New Desc: %d \n", fd);
    matrixX -> filePtr = fdopen(fd, "w+");
    printf("New Desc: %d \n", fileno(matrixX -> filePtr));


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
        fprintf(matrixX -> filePtr, "%d ", colStart);
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

// TODO fix synchronization - (kill me ...)
void commonFileManager(Matrix* matrixA, Matrix* matrixB, char* resultFilename, int workersNum, int maxTime) {
    Matrix* matrixX = createResultFile(resultFilename, matrixA -> rows, matrixB -> cols);

    int* workersPids  = malloc(sizeof(int) * workersNum);
    double runningSum = 0.0;
    double factor     = ((double)  matrixB -> cols ) / workersNum;

    for (int i = 0; i < workersNum; i++) {
        int start = (int) runningSum;
        runningSum += factor;
        int end   = (int) runningSum;

        int forked = fork();
        if (forked == 0) { 
            if (i == workersNum - 1) 
                end = matrixB -> cols;
            
            runWorker(matrixA, matrixB, matrixX, resultFilename, maxTime, start, end);
        } else { 
            workersPids[i] = forked;    
        }
    }

    for (int i = workersNum - 1; i >= 0; i--) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    fclose(matrixA -> filePtr);
    fclose(matrixB -> filePtr);
    fclose(matrixX -> filePtr);

    free(matrixA);
    free(matrixB);
    free(matrixX);
    free(workersPids);
}
// -----

int main(int argc, char* argv[]) {
    if (argc < 5) 
        error("Not enaugh arguments");

    char* configFile = argv[1];
    int workersNum   = atoi(argv[2]);
    int timeLimit    = atoi(argv[3]);
    char* executionFlag = argv[4];
    
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

    if (strcmp(executionFlag, "-commonFile") == 0) {
        commonFileManager(matrixA, matrixB, resultFilename, workersNum, timeLimit);
    } else if(strcmp(executionFlag, "-distinctFiles") == 0) {
        distinctFilesManager(matrixA, matrixB, resultFilename, workersNum, timeLimit);
    } else {
        error("Given flag is not supported. Use '-commonFile' or '-distinctFiles'");
    }
    
    return 0;
}