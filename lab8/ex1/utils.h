#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

struct PGM {
  int width;
  int height;
  int M;
} typedef PGM;

int** readImg(const char* filename, PGM* header);
void freeImg(int** img, PGM* header);
int readNext(FILE* ptr);
bool stringEq(const char* x, const char* y);
void printResults(int* data, int m);
void saveResults(int* results, int m, const char* filename);

