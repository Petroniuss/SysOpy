#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
#include <linux/limits.h>
#include "utils_lib.h"
#include "matrix_lib.h"

int* row;
int* col;

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

    int n = matrixA -> cols;
    row = malloc(sizeof(int) * n);
    col = malloc(sizeof(int) * n);

    Matrix* matrixX = createResultFile(resultFilename, matrixA -> rows, matrixB -> cols);

    int* workersPids = malloc(sizeof(int) * workersNum);

    for (int i = 0; i < workersNum; i++) {
        int forked = fork();
        if (forked == 0) { //child
            // Should probably do some stuff..
            return 1;
        } else { // parent
            workersPids[i] = forked;    
        }
    }

    for (int i = 0; i < workersNum; i++) {
        int returnStatus;
        waitpid(workersPids[i], &returnStatus, 0);
        printf("Process %d ended with status: %d\n", workersPids[i], WEXITSTATUS(returnStatus));
    }

    return 0;
}

