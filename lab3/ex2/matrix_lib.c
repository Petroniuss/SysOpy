#include <stdio.h>
#include <stdlib.h>
#include "utils_lib.h"

struct Matrix {
    FILE* filePtr;
    int rows;
    int cols;
} typedef Matrix;

int countElemsInFirstRow(FILE* filePtr) {
    int spaces = 0;
    movePtrToLine(filePtr, 0);

    for (char c = getc(filePtr); c != '\n'; c = getc(filePtr)) {
        if (c == ' ')  
            spaces++;
    }

    return spaces + 1;
}

void readRow(Matrix* matrix, int* nums, int row) {
    movePtrToLine(matrix -> filePtr, row);
    for (int i = 0; i < matrix -> cols; i++) {
        fscanf(matrix -> filePtr, "%d", &(nums[i]));
    }
}

void readNextRow(Matrix* matrix, int* nums) {
    for (int i = 0; i < matrix -> cols; i++) {
        fscanf(matrix -> filePtr, "%d", &(nums[i]));
    }
}

void readColumn(Matrix* matrix, int* nums, int col) {
    movePtrToLine(matrix -> filePtr, 0);

    for (int i = 0; i < matrix -> rows; i++) {
        int j = 0;
        int num;

        while (j <= col) {
            fscanf(matrix -> filePtr, "%d", &num);
            j++;
        }

        nums[i] = num;
        movePtrToNextLine(matrix -> filePtr);
    }
}

Matrix* createResultFile(char* filename, int rows, int cols) {
    Matrix* matrix = malloc(sizeof(Matrix));

    matrix -> filePtr = fopen(filename, "w+");
    matrix -> rows    = rows;
    matrix -> cols    = cols;

    for(int j = 0; j < rows; j++) {
        for(int i = 1; i < cols; i++) {
            fputc(' ', matrix -> filePtr);
        }
        fputc('\n', matrix -> filePtr);
    }

    fflush(matrix -> filePtr);

    return matrix;
}

void writeResult(Matrix* matrixX, int row, int col, int res) {
    movePtrToLine(matrixX -> filePtr, row);

    int spaces = 0;
    while (spaces < col) {
        if (getc(matrixX -> filePtr) == ' ')
            spaces++;
    }
    
    char* strNum = numberToString(res);

    finsert(matrixX -> filePtr, strNum);
    fflush(matrixX -> filePtr);

    free(strNum);
}


int dotVectors(int* row, int* col, int n) {
    int result = 0;

    for (int i = 0; i < n; i++) {
        result += row[i] * col[i];
    }

    return result;
}