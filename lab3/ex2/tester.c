#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <linux/limits.h>
#include "utils_lib.h"
#include "matrix_lib.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MIN -2
#define MAX 2
#define TES_DIR "test/"
#define MAX_EXEC_TIME "5"
#define EXECUTION_FLAG "-commonFile"

int generateTest(int min, int max, int testNum) {
    char filename1 [PATH_MAX];
    char filename2 [PATH_MAX];
    char filename3 [PATH_MAX];
    char configFilename [PATH_MAX];

    printf("---------------------------------------\n");
    printf("Test %d\n", testNum);

    sprintf(filename1, "%s%s-%d.tst", TES_DIR, "matrixA", testNum);
    sprintf(filename2, "%s%s-%d.tst", TES_DIR, "matrixB", testNum);
    sprintf(filename3, "%s%s-%d.tst", TES_DIR, "matrixX", testNum);
    sprintf(configFilename, "%s%s-%d.tst", TES_DIR, "args", testNum);

    FILE* conf = fopen(configFilename, "w");
    fprintf(conf, "%s %s %s", filename1, filename2, filename3);
    fclose(conf);

    int m = randNum(min, max);
    int n = randNum(min, max);
    int k = randNum(min, max);

    Matrix* matrixA = generateMatrix(filename1, m, n, MIN, MAX);
    Matrix* matrixB = generateMatrix(filename2, n, k, MIN, MAX);

    printf("\tGenerated test matrices\n");
    
    int pid = vfork();
    if (pid == 0) {
        int workers = randNum(1, k);
        char* workersStr = numberToString(workers);
        printf("\tRunning parallel... with %d workers and %ss maximum execution time, matrixA: %dx%d, matrixB: %dx%d\n",
                         workers, MAX_EXEC_TIME, m, n, n, k);
                         
        execl("./parallel", "./parallel", configFilename, workersStr, MAX_EXEC_TIME, EXECUTION_FLAG, NULL);
        
        exit(1);
    } else {
        int returnStatus;
        printf("\tTesting...\n");
        waitpid(pid, &returnStatus, 0);

        if (returnStatus != 0) {
            printf("\tRunning failed\n");
            return 0;
        }
    }

    // Check if results are succesfull
    matrixA -> filePtr = fopen(filename1, "r");
    matrixB -> filePtr = fopen(filename2, "r");
    FILE* result       = fopen(filename3, "r");

    if (!result) {
        printf("\tTest failed.\n");
            return 0;
    }

    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    for (int i = 0; i < m; i++) {
        readRow(matrixA, row, i);
        for (int j = 0; j < k; j++) {
            readColumn(matrixB, col, j);

            int expected = dotVectors(row, col, n);
            int actual;
            fscanf(result, "%d", &actual);

            if (expected != actual) {
                printf("\tTest failed\n");
                printf("\tExpected %d and got %d on row: %d, column: %d\n", expected, actual, i, j);
                return 0;
            }
        }
    }

    free(row);
    free(col);
    fclose(matrixA -> filePtr);
    fclose(matrixB -> filePtr);
    fclose(result);

    printf("\tTest passed.\n");
    
    return 1;
}

int main(int argc, char* argv[]) {
    int testNumber = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);

    srand(time(NULL));
    int passed = 0;

    for (int i = 1; i <= testNumber; i++) {
        passed += generateTest(min, max, i);
    }

    printf("\nFinished testing - %d/%d passed.\n", passed, testNumber);
    printf("---------------------------------------\n");

    return 0;
}