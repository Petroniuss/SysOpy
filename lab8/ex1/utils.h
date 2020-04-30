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

PGM* readHeaderPGM(const char* filename);
FILE* init(const char* filename);
int readNext(FILE* ptr);
int readKth(FILE* ptr, int k);
void ignoreK(FILE* ptr, int k);
bool stringEq(const char* x, const char* y);
void printResults(int* data, int m);
void saveResults(int* results, int m, const char* filename);
