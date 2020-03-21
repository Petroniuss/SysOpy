#include <stdio.h>

struct Matrix {
    FILE* filePtr;
    int rows;
    int cols;
} typedef Matrix;

int countElemsInFirstRow(FILE* fileptr);

void readRow(Matrix* matrix, int* nums, int row);

void readNextRow(Matrix* matrix, int* nums);

void readColumn(Matrix* matrix, int* nums, int col);

Matrix* createResultFile(char* filename, int rows, int cols);

void writeResult(Matrix* matrixX, int row, int col, int res);

int dotVectors(int* row, int* col, int n);

Matrix* generateMatrix(char* filename, int rows, int cols, int min, int max);