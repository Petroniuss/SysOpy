#include <stdio.h>

struct Matrix {
    FILE* filePtr;
    int rows;
    int cols;
} typedef Matrix;

// String utils 
char* concat(const char* s1, const char* s2);

char* concatWithSeparator(const char* s1, const char* s2, const char* separator);

char* numberToString(int i);

int startsWithDigit(const char* str);

int isEmpty(const char* str);

char* copyString(const char* str);

int startsWith(const char* str, const char* prefix);

char* randomString(int length);

char* substring(char* str, int start, int end);

char* suffix(char* str, int from); 

// File uitls
int finsert(FILE* file, const char *buffer);

int countLines(FILE* filePtr);

void movePtrToLine(FILE* ptr, int line);

// Matrix --
int countElemsInFirstRow(FILE* fileptr);

void readRow(Matrix* matrix, int* nums, int row);

void readNextRow(Matrix* matrix, int* nums);

void readColumn(Matrix* matrix, int* nums, int col);

Matrix* createResultFile(char* filename, int rows, int cols);

void writeResult(Matrix* matrixX, int row, int col, int res);
// ----------