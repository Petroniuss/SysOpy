#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "utils_lib.h"

void error(char* msg) {
    printf("Error: %s \n", msg);
    exit(0);
}

Matrix* initMatrix(char filename[PATH_MAX]) {
    Matrix* matrix = malloc(sizeof(Matrix));
    
    matrix -> filePtr = fopen(filename, "r");
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

    return 0;
}
