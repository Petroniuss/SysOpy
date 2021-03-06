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
#include <sys/resource.h>

#include "utils_lib.h"
#include "matrix_lib.h"

void error(char* msg) {
    printf("Error: %s \n", msg);
    exit(1);
}

void runSimpleWorker(Matrix* matrixA, Matrix* matrixB, int maxTime, int colStart, int colEnd, char* outputFile, int cpuTimeLimit, int virtualMemoryLimit) {
    FILE* ptr = fopen(outputFile, "w");

    long int memoryLimitBytes = 1000000 * virtualMemoryLimit;

    // Set hard limits for cpu-time 
    struct rlimit cpuRL;
    getrlimit (RLIMIT_CPU, &cpuRL);

    cpuRL.rlim_max = cpuTimeLimit;
    setrlimit(RLIMIT_CPU, &cpuRL);

    // Set hard limits for virtual-memory-space
    struct rlimit memRL;
    getrlimit(RLIMIT_AS, &memRL);
    
    memRL.rlim_max = memoryLimitBytes;
    setrlimit(RLIMIT_AS, &memRL);

    int n = matrixA -> cols;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    int finishedMultiplications = 0;
    struct timespec start = nowRealTime();
    int pid = getpid();

    int rowCounter = 0;
    int colCounter = colStart;
    
    readRow(matrixA, row, 0);
    readColumn(matrixB, col, colStart);

    while(rowCounter < matrixA -> rows && realTime(start) < maxTime) {
        if (finishedMultiplications % 3 == 0)
            logSystemUsage(RUSAGE_SELF, pid);
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
    logSystemUsage(RUSAGE_SELF, pid);

    exit(finishedMultiplications);
}

void distinctFilesManager(Matrix* matrixA, Matrix* matrixB, char* resultFilename, int workersNum, int maxTime, int cpuTimeLimit, int virtualMemoryLimit) {
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
            
            runSimpleWorker(matrixA, matrixB, maxTime, start, end, files[i], cpuTimeLimit, virtualMemoryLimit);
        } else { 
            workersPids[i] = forked;    
        }
    }

    for (int i = workersNum - 1; i >= 0; i--) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    printf("Children usage\n");
    logSystemUsage(RUSAGE_CHILDREN, getpid());

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

    printf("Children usage\n");
    logSystemUsage(RUSAGE_CHILDREN, getpid());

    exit(0);
}

// COMMON FILE WORKERS!
void runWorker(char* filenameA, char* filenameB, char* resultFilename, int colStart, int colEnd, int maxTime, int cpuTimeLimit, int virtualMemoryLimit) {
    execl("./worker", "./worker", filenameA, filenameB, resultFilename, numberToString(colStart),
             numberToString(colEnd), numberToString(maxTime), numberToString(cpuTimeLimit), numberToString(virtualMemoryLimit), NULL);
}

void commonFileManager(Matrix* matrixA, Matrix* matrixB, char* filenameA, char* filenameB, char* resultFilename, int workersNum, int maxTime, int cpuTimeLimit, int virtualMemoryLimit) {
    Matrix* matrixX = createResultFile(resultFilename, matrixA -> rows, matrixB -> cols);
    fclose(matrixX -> filePtr);
    fclose(matrixA -> filePtr);
    fclose(matrixB -> filePtr);

    int* workersPids  = malloc(sizeof(int) * workersNum);
    double runningSum = 0.0;
    double factor     = ((double)  matrixB -> cols ) / workersNum;

    for (int i = 0; i < workersNum; i++) {
        int start = (int) runningSum;
        runningSum += factor;
        int end   = (int) runningSum;

        int forked = fork();
        if (forked == 0) {
            printf("Child"); 
            if (i == workersNum - 1) 
                end = matrixB -> cols;
            runWorker(filenameA, filenameB, resultFilename, start, end, maxTime, cpuTimeLimit, virtualMemoryLimit);
        } else { 
            workersPids[i] = forked;    
        }
    }

    // sleep(maxTime);

    for (int i = workersNum - 1; i >= 0; i--) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    free(matrixA);
    free(matrixB);
    free(matrixX);
    free(workersPids);
}
// -----

int main(int argc, char* argv[]) {
    if (argc < 7) 
        error("Not enaugh arguments");

    char* configFile = argv[1];
    int workersNum   = atoi(argv[2]);
    int timeLimit    = atoi(argv[3]);
    char* executionFlag = argv[4];
    int  cpuTimeLimit       = atoi(argv[5]);
    int  virtualMemoryLimit = atoi(argv[6]); 
    
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

    printf("Setting limits for child processes: CPU Time hard limit: %ds virtual memory hard limit: %dMb\n", cpuTimeLimit, virtualMemoryLimit);

    if (strcmp(executionFlag, "-commonFile") == 0) {
        commonFileManager(matrixA, matrixB, filenameA, filenameB, resultFilename, workersNum, timeLimit, cpuTimeLimit, virtualMemoryLimit);
    } else if(strcmp(executionFlag, "-distinctFiles") == 0) {
        distinctFilesManager(matrixA, matrixB, resultFilename, workersNum, timeLimit, cpuTimeLimit, virtualMemoryLimit);
    } else {
        error("Given flag is not supported. Use '-commonFile' or '-distinctFiles'");
    }
    
    return 0;
}